//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/channel_service.hpp>
#include <quippy/connector.hpp>
#include <quippy/channel_service.hpp>

namespace quippy {

    struct channel : detail::io_object<channel_service>
    {
        using service_type = channel_service;
        using implementation_type = service_type::implementation_type;

        using base_class = detail::io_object<channel_service>;

        channel(asio::io_service &owner) : base_class(owner) {}

        channel(connector& con)
        : base_class(con.get_io_service(), con.get_implemenentation())
        {

        }
    };

}