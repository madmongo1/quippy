//
// Created by Richard Hodges on 21/04/2017.
//

#include <quippy/detail/connection.hpp>
#include <quippy/detail/connection_handler.hpp>

namespace quippy { namespace detail {

    asio_connection::asio_connection(asio_connection_handler &handler)
        : handler_(handler), socket_(handler_.get_worker_executor()) {

    }

        void asio_connection::async_connect_link(asio::ip::tcp::resolver::iterator iter,
                                std::function<void(const asio::error_code&)> handler)
        {
        assert(state_ == state_type::link_down);
        state_ = state_type ::link_connecting;
        asio::async_connect(socket_, iter, [this, handler](auto const& ec, auto iter) mutable {
            this->handle_connect(ec, iter, std::move(handler));
        });

    }

    void asio_connection::handle_connect(asio::error_code const& ec,
                                         asio::ip::tcp::resolver::iterator iter,
                                         std::function<void(const asio::error_code&)> handler)
    {
        if (ec)
        {
            state_ = state_type::link_down;

        }
        else {
            state_ = state_type::link_up;
        }
        handler(ec);
    }


    void asio_connection::check_for_send() {
        if (sending_data_.size() == 0 and pending_send_data_.size() != 0) {
            std::swap(pending_send_data_, sending_data_);
            async_write(socket_, asio::buffer(sending_data_), [this](auto const &ec, auto size) {
                this->handle_send(ec, size);
            });
        }
    }

    void asio_connection::handle_send(asio::error_code const &ec, std::size_t sent) {
        if (ec) {
            handle_error(ec);
        } else {
            check_for_send();
        }
    }

    void asio_connection::handle_error(asio::error_code const &ec) {

    }


}}