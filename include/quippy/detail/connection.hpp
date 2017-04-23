//
// Created by Richard Hodges on 21/04/2017.
//

#pragma once
#include <amqpcpp.h>
#include <quippy/config.hpp>
#include <functional>
#include <boost/optional.hpp>

namespace quippy { namespace detail {

    struct asio_connection_handler;

    struct asio_connection
    {
        using completion_function = std::function<void(asio::error_code const&)>;

        asio_connection(asio_connection_handler& handler);

        void async_connect_link(asio::ip::tcp::resolver::iterator iter,
                                completion_function handler);

        void async_connect(AMQP::Login const& login, std::string const& vhost,
                           completion_function handler);

        void queue_send(const char* data, std::size_t length)
        {
            pending_send_data_.insert(std::end(pending_send_data_), data, data + length);
            check_for_send();
        }

        void notify_connected();

        void notify_disconnected();

        void notify_error(const char* message);


        void notify_stop()
        {

        }

        asio::io_service& get_io_service() {
            return socket_.get_io_service();
        }

        auto get_connection_handler() -> asio_connection_handler&
        {
            return handler_;
        }

        auto get_connection_handler_ptr() -> asio_connection_handler*
        {
            return std::addressof(get_connection_handler());
        }

        auto get_amqp_key() -> AMQP::Connection* {
            assert(amqp_connection_.is_initialized());
            return amqp_connection_.get_ptr();
        }


    private:
        void check_for_send();

        void handle_connect_link(asio::error_code const& ec,
                            asio::ip::tcp::resolver::iterator iter,
                            std::function<void(const asio::error_code&)> handler);
        void handle_send(asio::error_code const& ec, std::size_t sent);
        void handle_error(asio::error_code const& ec);

        void initiate_read();
        void handle_read(asio::error_code const& ec, std::size_t size);

        enum state_type {
            link_down,
            link_connecting,
            link_up,
            amqp_connecting,
            amqp_connected,
            error,
        };


        asio_connection_handler& handler_;
        asio::io_service& worker_executor_;
        asio::ip::tcp::socket socket_ { worker_executor_ };
        std::vector<char> pending_send_data_ {};
        std::vector<char> sending_data_ {};
        state_type state_ = link_down;

        // data for amqp comms states
        boost::optional<AMQP::Connection> amqp_connection_;
        completion_function connected_function_;

        asio::streambuf read_buffer_;
        static const std::size_t read_capacity = 4096;
    };
}}