//
// Created by Richard Hodges on 21/04/2017.
//

#include <quippy/detail/connection.hpp>
#include <quippy/detail/connection_handler.hpp>

namespace quippy { namespace detail {
/*
    asio_connection::asio_connection(asio_connection_handler &handler)
        : handler_(handler)
    , worker_executor_(handler_.get_worker_executor())
    {
    }

    void asio_connection::async_connect_link(asio::ip::tcp::resolver::iterator iter,
                                             std::function<void(const asio::error_code &)> handler) {
        assert(state_ == state_type::link_down);
        state_ = state_type::link_connecting;
        asio::async_connect(socket_, iter, [this, handler](auto const &ec, auto iter) mutable {
            this->handle_connect_link(ec, iter, std::move(handler));
        });

    }

    void asio_connection::handle_connect_link(asio::error_code const &ec,
                                         asio::ip::tcp::resolver::iterator iter,
                                         std::function<void(const asio::error_code &)> handler) {
        if (ec) {
            state_ = state_type::link_down;

        } else {
            state_ = state_type::link_up;
        }
        handler(ec);
    }

    void asio_connection::async_connect(AMQP::Login const &login, std::string const &vhost,
                                        completion_function handler) {
        assert(state_ == state_type::link_up);
        if (state_ != state_type::link_up)
            handler(asio::error::not_connected);
        else {
            state_ = state_type::amqp_connecting;
            amqp_connection_.emplace(get_connection_handler_ptr(), login, vhost);
            connected_function_ = std::move(handler);
            handler_.bingo(this);
            initiate_read();
        }
    }

    void asio_connection::initiate_read()
    {
        auto buffers = read_buffer_.prepare(read_capacity);
        socket_.async_read_some(buffers, [this](auto const& ec, auto size) {
            this->read_buffer_.commit(size);
            this->handle_read(ec, size);
        });
    }

    void asio_connection::handle_read(asio::error_code const& ec, std::size_t size)
    {
        auto buffer = read_buffer_.data();
        auto parsed = amqp_connection_.get().parse(asio::buffer_cast<const char*>(buffer),
                                                   asio::buffer_size(buffer));
        read_buffer_.consume(parsed);
        if (ec) {
            // handle this
        }
        else {
            initiate_read();
        }
    }

    void asio_connection::notify_connected()
    {
        switch(state_)
        {
            case state_type::amqp_connecting: {
                state_ = state_type::amqp_connected;
                auto cb = std::move(connected_function_);
                connected_function_ = nullptr;
                cb(asio::error_code());
            } break;

            default:
                assert(!"handle me");
        }
    }

    void asio_connection::notify_disconnected()
    {
        switch(state_)
        {
            case state_type ::amqp_connecting: {
                state_ = state_type::link_up;
                auto cb = std::move(connected_function_);
                connected_function_ = nullptr;
                cb(asio::error_code(asio::error::connection_refused));
            } break;
            default:
                assert(!"handle me");
        }
    }

    void asio_connection::notify_error(const char* message)
    {
        switch(state_) {
            case state_type::amqp_connecting: {
                auto cb = std::move(connected_function_);
                connected_function_ = nullptr;
                cb(asio::error_code(asio::error::connection_refused));
                // DEPENDENCY: ORDER - the amqp_connection_ must exist
                // DEPENDENCY: ORDER - we must still be in the link up state during unbingo
                handler_.unbingo(this);
                amqp_connection_.reset();
                state_ = state_type::link_up;
                asio::error_code sink;
                socket_.cancel(sink);
            } break;

            default:
                assert(!"handle me");
        }
    }




    void asio_connection::check_for_send() {
        std::cerr << __func__ << " pending: " << pending_send_data_.size() << " sending: " << sending_data_.size() << std::endl;
        if (sending_data_.size() == 0 and pending_send_data_.size() != 0) {
            std::swap(pending_send_data_, sending_data_);

            std::cerr << __func__ << " size: " << sending_data_.size() << std::endl;

            async_write(socket_, asio::buffer(sending_data_), [this](auto const &ec, auto size) {
                this->handle_send(ec, size);
            });
        }
    }

    void asio_connection::handle_send(asio::error_code const &ec, std::size_t sent) {
        std::cerr << __func__ << " size: " << sent << " " << "error: " << ec.message() << std::endl;
        if (ec) {
            handle_error(ec);
        } else {
            sending_data_.clear();
            check_for_send();
        }
    }

    void asio_connection::handle_error(asio::error_code const &ec) {

    }

*/
}}