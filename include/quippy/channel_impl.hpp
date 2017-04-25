//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once
#include <quippy/config.hpp>
#include <quippy/connector_impl.hpp>

namespace quippy {

    struct channel_impl
    {
        channel_impl(connector_impl connector_ref)
        : connector_ref_(std::move(connector_ref))
        {

        }


        connector_impl connector_ref_;
    };

}