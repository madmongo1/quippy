//
// Created by Richard Hodges on 21/04/2017.
//

#pragma once

#include <amqpcpp.h>
#include <quippy/config.hpp>
#include <unordered_map>

namespace quippy {

    namespace detail {

        struct connection_handler_target {

            struct concept {
                virtual void onData(const char *buffer, size_t size) = 0;

                virtual void onConnected() = 0;

                virtual void onClosed() = 0;

                virtual void onError(const char *message) = 0;
            };

            template<class BackEnd>
            struct model : concept {

            };

            std::unique_ptr<concept> impl_;

            connection_handler_target(std::unique_ptr<concept>&& impl)
                : impl_(std::move(impl))
            {}

            connection_handler_target()
                : impl_()
            {}

            void onData(const char *buffer, size_t size)
            {
                assert(impl_.get());
                impl_->onData(buffer, size);
            }

            void onConnected()
            {
                assert(impl_.get());
                impl_->onConnected();
            }

            void onClosed()
            {
                assert(impl_.get());
                impl_->onClosed();
            }

            void onError(const char *message) {
                assert(impl_.get());
                impl_->onError(message);
            }

            bool empty() const {
                return not impl_.get();
            }

        };


        struct asio_connection_handler : AMQP::ConnectionHandler {
            asio_connection_handler(asio::io_service &worker_executor);

            asio_connection_handler(const asio_connection_handler &) = delete;

            void onData(AMQP::Connection *connection, const char *buffer, size_t size) override;

            void onConnected(AMQP::Connection *connection) override;

            void onClosed(AMQP::Connection *connection) override;

            void onError(AMQP::Connection *connection, const char *message) override;

            auto get_worker_executor() const -> asio::io_service & { return worker_executor_; }


            void bingo(AMQP::Connection *key, connection_handler_target&& target);

            void unbingo(AMQP::Connection *connection);

            asio::io_service &worker_executor_;

            struct connection_info {
                std::vector<char> deferred_data = {};
                connection_handler_target connection;
            };

            static void flush_data(connection_info &ci, const char *data = nullptr, std::size_t size = 0);

            // use the AMQP::Connection* as a handle to identify the amqp_connection
            std::unordered_map<AMQP::Connection *, connection_info> connections_;
        };
    }
}