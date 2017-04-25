//
// Created by Richard Hodges on 22/04/2017.
//

#pragma once
#include <quippy/config.hpp>
#include <functional>

namespace quippy
{
    using completion_handler_sig = void(asio::error_code const &ec);
    using completion_handler_function = std::function<completion_handler_sig>;

    struct has_completion_handlers {

        void fire_completion_handlers(asio::error_code const &ec) {
            auto copy = std::move(completion_handlers_);
            completion_handlers_.clear();
            for (auto &&handler : copy) {
                handler(ec);
            }
        }

        template<class F>
        void add_completion_handler(F&& f) {
            completion_handlers_.emplace_back(std::forward<F>(f));
        }

        std::vector<completion_handler_function> completion_handlers_;
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