//
// Created by Richard Hodges on 20/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/connector_impl.hpp>
#include <functional>
#include <unordered_map>

#include <quippy/detail/connection_handler.hpp>
#include <quippy/detail/connection.hpp>
#include <quippy/detail/result_or_error.hpp>
#include <quippy/connector_impl.hpp>

namespace quippy {

    struct connector_service : asio::detail::service_base<connector_service> {
        using implementation_class = connector_impl;
        using implementation_type = std::shared_ptr<connector_impl>;

        connector_service(asio::io_service &owner)
            : asio::detail::service_base<connector_service>(owner)
        , worker_executor_()
        , worker_executor_work_(worker_executor_)
        , connection_handler_(worker_executor_)
        , worker_thread_()
        {
            worker_thread_ = std::thread([this]() {
                while (not this->worker_executor_.stopped()) {
                    this->worker_executor_.run_one();
                }
            });
        }

        implementation_type create() {
            auto impl = std::make_shared<implementation_class>(boost::ref(connection_handler_));
            impl->start();
            return impl;
        }

        void destroy(implementation_type &impl) {
            detail::result_or_error<void> result;
            auto handler = [&](auto const &ec) {
                result.set_value_or_error(ec);
            };

            impl->notify_event(event_halt {handler});

            result.wait();
            impl.reset();
        }

        void connect_link(implementation_class &impl, asio::ip::tcp::resolver::iterator iter) {
            detail::result_or_error<void> result;

            impl.notify_event(implementation_class::event_connect_tcp{
                iter,
                [&](const asio::error_code &ec) {
                    result.set_value_or_error(ec);
                }});

            return result.get();
        }

        void connect(implementation_class &impl, AMQP::Login const& login, std::string const& vhost) {
            detail::result_or_error<void> result;

            impl.notify_event(implementation_class::event_connect_protocol {
                login,
                vhost,
                [&](auto &&ec) { result.set_value_or_error(ec); }
            });

            return result.get();
        }

        void add_connection() {
            auto conn_ptr = std::make_unique<detail::asio_connection>(connection_handler_);
        }


        auto get_worker_executor() -> asio::io_service & {
            return worker_executor_;
        }

    private:

        auto shutdown_service() -> void override {
            worker_executor_.stop();
            if (worker_thread_.joinable())
                worker_thread_.join();
        }

        asio::io_service worker_executor_;
        asio::io_service::work worker_executor_work_;
        detail::asio_connection_handler connection_handler_;
        std::thread worker_thread_;
    };


}