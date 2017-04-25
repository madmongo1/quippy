//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/handlers.hpp>
#include <quippy/connector_impl/traits.hpp>

namespace quippy {

    struct event_connect_tcp
    : expects_completion
    , connector_impl_traits
    {
        event_connect_tcp(tcp::resolver::iterator iter, completion_handler_function completion_handler)
        : expects_completion(std::move(completion_handler))
        , iter(std::move(iter)) {}

        tcp::resolver::iterator iter;
    };

    struct event_connect_error
    {
        asio::error_code error;
    };

}