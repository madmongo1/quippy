//
// Created by Richard Hodges on 21/04/2017.
//

#pragma once
#include <amqpcpp.h>
#include <quippy/config.hpp>
#include <functional>

namespace quippy { namespace detail {

    struct asio_connection_handler;

    struct asio_connection
    {
        asio_connection(asio_connection_handler& handler);

        void async_connect_link(asio::ip::tcp::resolver::iterator iter,
                                std::function<void(const asio::error_code&)> handler);

        void queue_send(const char* data, std::size_t length)
        {
            pending_send_data_.insert(std::end(pending_send_data_), data, data + length);
            check_for_send();
        }

        void notify_stop()
        {

        }

    private:
        void check_for_send();

        void handle_connect(asio::error_code const& ec,
                            asio::ip::tcp::resolver::iterator iter,
                            std::function<void(const asio::error_code&)> handler);
        void handle_send(asio::error_code const& ec, std::size_t sent);
        void handle_error(asio::error_code const& ec);

        enum state_type {
            link_down,
            link_connecting,
            link_up,
            error,
        };


        asio_connection_handler& handler_;
        asio::ip::tcp::socket socket_;
        std::vector<char> pending_send_data_;
        std::vector<char> sending_data_;
        state_type state_ = link_down;

    };
}}