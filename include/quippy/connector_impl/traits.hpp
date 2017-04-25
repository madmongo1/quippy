//
// Created by Richard Hodges on 22/04/2017.
//

#pragma once
#include <quippy/config.hpp>
#include <quippy/handlers.hpp>

namespace quippy
{
    struct connector_impl_traits {
        using tcp = asio::ip::tcp;
    };

    struct event_halt : connector_impl_traits {
        operator asio::error_code() const {
            return make_error_code(asio::error::operation_aborted);
        }
    };

    struct expects_completion
    {
        template<class Handler>
        expects_completion(Handler&& handler)
            : completion_handler(std::forward<Handler>(handler))
        {}

        completion_handler_function completion_handler;
    };


}