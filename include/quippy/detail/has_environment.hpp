//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>

namespace quippy { namespace detail {

    template<class Environment>
    struct has_environment {
        using environment_type = Environment;

        has_environment(environment_type *penv = nullptr)
            : penv_(penv) {}

        void set_environment(environment_type& env) {
            // can only set the same environment
            auto penv = std::addressof(env);
            assert(penv_ == nullptr or (penv == penv_));
            penv_ = penv;
        }

        environment_type &get_environment() {
            assert(penv_);
            return *penv_;
        }

        environment_type const &get_environment() const {
            assert(penv_);
            return *penv_;
        }

        friend environment_type &environment_of(has_environment &impl) { return impl.get_environment(); }

        friend environment_type const &environment_of(has_environment const &impl) { return impl.get_environment(); }

    private:
        environment_type *penv_;
    };

}}