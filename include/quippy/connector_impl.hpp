//
// Created by Richard Hodges on 20/04/2017.
//

#pragma once
#include <quippy/detail/connection_handle.hpp>
#include <thread>
#include <mutex>
#include <deque>
#include <amqpcpp.h>

namespace quippy {


    struct connector_impl
    {

        void notify_stop()
        {

        }

    private:

        enum state_type {
            not_started,
            connecting,
            connected,
            finished
        };

        state_type state_ = not_started;


    };
}
