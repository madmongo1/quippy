//
// Created by Richard Hodges on 22/04/2017.
//

#pragma once
#include <quippy/config.hpp>
#include <quippy/sm.hpp>
#include <quippy/handlers.hpp>

namespace quippy
{
    struct connector_impl_traits {
        using tcp = asio::ip::tcp;
        using halted_signal_type = sig::signal<void()>;
        using halted_slot_type = halted_signal_type::slot_type;
    };




}