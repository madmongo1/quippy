//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/sm.hpp>

namespace quippy {

    struct event_connect_protocol
    : expects_completion, connector_impl_traits
    {
        event_connect_protocol(AMQP::Login const &login,
                               std::string const &vhost,
                               completion_handler_function &&f)
        : expects_completion(std::move(f))
        , login(std::move(login))
        , vhost(std::move(vhost)) {}

        AMQP::Login login;
        std::string vhost;
    };

}