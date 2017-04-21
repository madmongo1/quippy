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

namespace quippy {

    struct connector_service : asio::detail::service_base<connector_service>
    {
        using implementation_class = detail::asio_connection;

        struct deleter
        {
            void operator()(implementation_class* impl) const {
                impl->notify_stop();
                delete impl;
            }
        };

        using implementation_type = std::unique_ptr<implementation_class, deleter>;

        connector_service(asio::io_service& owner)
            : asio::detail::service_base<connector_service>(owner)
        {
            worker_thread_ = std::thread([this](){
                while(not this->worker_executor_.stopped()) {
                    this->worker_executor_.run_one();
                }
            });
        }

        implementation_type create()
        {
            auto impl = implementation_type { new detail::asio_connection(connection_handler_),
            deleter()
            };
            return impl;
        }

        void connect(implementation_type& impl, asio::ip::tcp::resolver::iterator iter)
        {
            detail::result_or_error<void> result;
            impl->async_connect_link(iter, [&](const asio::error_code& ec) {
                if (ec) {
                    result.set_error(ec);
                }
                else {
                    result.set_value();
                }
            });
            return result.get();
        }

        void add_connection()
        {
            auto conn_ptr = std::make_unique<detail::asio_connection>(connection_handler_);
        }


    private:

        auto shutdown_service() -> void override {
            worker_executor_.stop();
            if (worker_thread_.joinable())
                worker_thread_.join();
        }

        asio::io_service worker_executor_;
        asio::io_service::work worker_executor_work_ { worker_executor_ };
        detail::asio_connection_handler connection_handler_ { worker_executor_ };
        std::thread worker_thread_;
    };


}