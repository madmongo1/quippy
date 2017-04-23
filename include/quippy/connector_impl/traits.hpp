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
        event_halt(completion_handler_function &&f)
            : handler(std::move(f)) {}

        void fire_completion_handler() const {
            handler(asio::error_code());
        }

        completion_handler_function handler;
    };


}