//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once
#include <quippy/connector_impl/traits.hpp>

namespace quippy
{
    struct event_halt : connector_impl_traits {
        operator asio::error_code() const {
            return make_error_code(asio::error::operation_aborted);
        }
    };

}