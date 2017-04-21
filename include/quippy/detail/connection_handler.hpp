//
// Created by Richard Hodges on 21/04/2017.
//

#pragma once

#include <amqpcpp.h>
#include <quippy/config.hpp>
#include <unordered_map>

namespace quippy { namespace detail {

    struct asio_connection;

    struct asio_connection_handler : AMQP::ConnectionHandler
    {
        asio_connection_handler(asio::io_service& worker_executor);

        void onData(AMQP::Connection *connection, const char *buffer, size_t size) override;

        auto get_worker_executor() const -> asio::io_service& { return worker_executor_; }

        asio::io_service& worker_executor_;

        // use the AMQP::Connection* as a handle to identify the amqp_connection
        std::unordered_map<AMQP::Connection*, asio_connection*> connections_;
    };
}}