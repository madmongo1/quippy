//
// Created by Richard Hodges on 21/04/2017.
//

#pragma once

#include <amqpcpp.h>
#include <quippy/config.hpp>
#include <unordered_map>

namespace quippy {

    struct connector_impl_;

    namespace detail {


    struct asio_connection_handler : AMQP::ConnectionHandler
    {
        asio_connection_handler(asio::io_service& worker_executor);
        asio_connection_handler(const asio_connection_handler&) = delete;

        void onData(AMQP::Connection *connection, const char *buffer, size_t size) override;

        void onConnected(AMQP::Connection *connection) override;

        void onClosed(AMQP::Connection *connection) override;

        void onError(AMQP::Connection *connection, const char* message) override;

        auto get_worker_executor() const -> asio::io_service& { return worker_executor_; }


        void bingo(AMQP::Connection* key, connector_impl_* connector);
        void unbingo(AMQP::Connection* connection);

        asio::io_service& worker_executor_;

        struct connection_info
        {
            std::vector<char> deferred_data = {};
            connector_impl_* connection = nullptr;
        };

        static void flush_data(connection_info& ci, const char* data = nullptr, std::size_t size = 0);

        // use the AMQP::Connection* as a handle to identify the amqp_connection
        std::unordered_map<AMQP::Connection*, connection_info> connections_;
    };
}}