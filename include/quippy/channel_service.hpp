//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/channel_impl.hpp>
#include <quippy/connector_service.hpp>

namespace quippy {

    struct channel_service : asio::detail::service_base<channel_service>
    {
        using implementation_class = channel_impl;
        using implementation_type = std::unique_ptr<implementation_class >;

        channel_service(asio::io_service& owner)
        : asio::detail::service_base<channel_service>(owner)
        {

        }

        implementation_type create()
        {
            return nullptr;
        }

        implementation_type create(connector_service::implementation_type conn)
        {
            return std::make_unique<implementation_class>(std::move(conn));
        }

        /// if impl exists, send the cancel message and wait for confirmation that it has been enqueued
        void cancel(implementation_type& impl)
        {

        }

        /// if impl exists, send the cancel message and wait for confirmation that it has been enqueued
        /// then destroy the impl so the handle can be re-used
        void destroy(implementation_type& impl)
        {
            // cancel
            // halt
            impl.reset();
        }

    private:
        void shutdown_service()
        {

        }
    };
}