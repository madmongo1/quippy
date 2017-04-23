//
// Created by Richard Hodges on 22/04/2017.
//

#include <quippy/connector_impl/transport_up.hpp>
#include <quippy/connector_impl.hpp>

namespace quippy
{


    void connector_impl_transport_up_::initiate_connect(const AMQP::Login& login, const std::string& vhost)
    {
        auto parent = get_parent();
        auto handler_ptr = std::addressof(parent->get_connection_handler());
        connection_.emplace(handler_ptr, login, vhost);
        handler_ptr->bingo(get_connection_ptr(), parent);
        start_receiving();

    }

    void connector_impl_transport_up_::add_send_data(std::vector<char> const &source) {
        send_buffer_.insert(std::end(send_buffer_),
                            std::begin(source),
                            std::end(source));
        maybe_send();
    }

    void connector_impl_transport_up_::maybe_send() {
        if (send_buffer_.size() and sending_buffer_.empty()) {
            auto parent = get_parent();

            auto lifetime = parent->shared_from_this();
            auto self = static_cast<msm::back::state_machine<connector_impl_transport_up_>*>(this);

            auto &socket = parent->socket();

            std::swap(send_buffer_, sending_buffer_);

            asio::async_write(socket, asio::buffer(sending_buffer_),
                              [lifetime, this, self](auto const &ec, auto size) {
                                  self->handle_send(ec, size);
                              });
        }
    }

    void connector_impl_transport_up_::handle_send(asio::error_code const& ec, std::size_t size)
    {
        sending_buffer_.erase(std::begin(sending_buffer_),
                              std::next(std::begin(sending_buffer_),
                                        size));

        send_buffer_.insert(std::begin(send_buffer_),
                            std::begin(sending_buffer_),
                            std::end(sending_buffer_));
        sending_buffer_.clear();
        if (ec) {
            auto self = static_cast<msm::back::state_machine<connector_impl_transport_up_>*>(this);
            self->process_event(ec);
        }
        else {
            maybe_send();
        }
    }



    void connector_impl_transport_up_::start_receiving()
    {
        auto parent = get_parent();
        auto& socket = parent->socket();

        auto lifetime = parent->shared_from_this();
        auto self = static_cast<msm::back::state_machine<connector_impl_transport_up_>*>(this);

        auto& connection = connection_.get();
        auto read_size = std::size_t(4096);
        read_size = std::max<std::size_t>(read_size, connection.expected());

        socket.async_read_some(receive_buffer_.prepare(read_size),
                               [lifetime, self](auto&&ec, auto size)
                               {
                                   self->receive_buffer_.commit(size);
                                   self->process_event(event_received_data(ec));
                               });

    }

    void connector_impl_transport_up_::do_receive_processing(event_received_data const& event)
    {
        std::cout << __func__ << " " << event.error.message() << std::endl;
        auto buffer = receive_buffer_.data();
        auto buffer_size = asio::buffer_size(buffer);
        auto data_needed = connection_.get().expected();

        if (data_needed <= buffer_size)
        {
            std::cout << __func__ << " parsing" << std::endl;
            auto used = connection_.get().parse(asio::buffer_cast<const char*>(buffer), buffer_size);
            receive_buffer_.consume(used);
            std::cout << __func__ << " parsed " << used << std::endl;
        }

        if (event.error) {
            get_parent()->notify_event(event.error);
        }
        else {
            start_receiving();
        }
    }

    void connector_impl_transport_up_::cancel_io()
    {
        auto& socket = get_parent()->socket();
        asio::error_code sink;
        socket.cancel(sink);
    }

    void connector_impl_transport_up_::unbingo(asio::error_code const& error)
    {
        auto parent = get_parent();
        auto& handler = parent->get_connection_handler();
        assert(bool(connection_));
        auto& connection = connection_.get();
        connection.notifyTransportError("transport error: " + error.message());
        handler.unbingo(connection_.get_ptr());
        connection_.reset();
        cancel_io();
    }

    void connector_impl_transport_up_::unbingo()
    {
        auto parent = get_parent();
        auto& handler = parent->get_connection_handler();
        assert(bool(connection_));
        auto& connection = connection_.get();
        connection.close();
        handler.unbingo(connection_.get_ptr());
        connection_.reset();
    }



}