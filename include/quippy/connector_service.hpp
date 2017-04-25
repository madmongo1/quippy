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
#include <quippy/notstd/apply.hpp>

namespace quippy {

    struct connector_service : asio::detail::service_base<connector_service> {
        using implementation_class = connector_impl;
        using implementation_type = std::shared_ptr<connector_impl>;
        using tcp = implementation_class::tcp;

        connector_service(asio::io_service &owner)
            : asio::detail::service_base<connector_service>(owner), worker_executor_(),
              worker_executor_work_(worker_executor_), connection_handler_(worker_executor_), worker_thread_() {
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
            halt(*impl);
            impl.reset();
        }

        void halt(implementation_class& impl)
        {
            detail::result_or_error<void> result;
            auto subs = subscribe_halted(impl, [&]() { result.set_value(); });
            impl.notify_event(implementation_class::event_halt());
            result.wait();
        }

        void connect_link(implementation_class &impl, asio::ip::tcp::resolver::iterator iter,
        asio::error_code& ec)
        {
            detail::signalled<asio::error_code> sig_ec;

            impl.notify_event(implementation_class::event_connect_tcp{
                iter,
                [&](const asio::error_code &ec) {
                    sig_ec.set_value(ec);
                }});

            sig_ec.visit([&ec](auto& value){ ec = std::forward<decltype(value)>(value); });
        }

        template<class Handler>
        auto make_async_handler(implementation_class &impl, Handler &&handler) {
            auto &executor = get_io_service();

            auto client_work = asio::io_service::work(executor);

            auto shared_impl = impl.shared_from_this();

            auto caller = [
                handler = std::forward<Handler>(handler),
                client_work,
                shared_impl
            ]
                (auto &&...args) {
                handler(args...);
            };

            return executor.wrap(caller);
        }

        template<class Handler>
        void async_connect_link(implementation_class &impl, tcp::resolver::iterator iter, Handler &&handler) {


            impl.notify_event(implementation_class::event_connect_tcp{
                iter,
                make_async_handler(impl,
                                   std::forward<Handler>(handler))
            });
        }

        void connect(implementation_class &impl, AMQP::Login const &login, std::string const &vhost, asio::error_code& ec) {
            detail::signalled<asio::error_code> sig_ec;

            impl.notify_event(implementation_class::event_connect_protocol {
                login,
                vhost,
                [&](auto &&ec) { sig_ec.set_value(ec); }
            });

            sig_ec.visit([&ec](auto& value){ ec = std::forward<decltype(value)>(value); });
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