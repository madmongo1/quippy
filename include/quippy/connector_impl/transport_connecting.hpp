//
// Created by Richard Hodges on 22/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/handlers.hpp>
#include <quippy/connector_impl.hpp>
#include <functional>
#include <vector>

namespace quippy {

    struct connection_impl_transport_connecting
    {
        using completion_function = std::function<void(asio::error_code const&)>;

        void dispatch_completion_handlers(asio::error_code const& ec)
        {
            auto copy = std::move(completion_handlers_);
            completion_handlers_.clear();
            for (auto&& f : copy)
            {
                f(ec);
            }
        }

        void add_completion_handler(completion_function f) {
            completion_handlers_.push_back(std::move(f));
        }

        std::vector<completion_function> completion_handlers_;

    };
}