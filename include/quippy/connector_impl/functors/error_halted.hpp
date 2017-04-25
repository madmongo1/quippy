//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/sm.hpp>
#include <quippy/error.hpp>

#include <quippy/event/halt.hpp>
#include <quippy/connector_impl/states/halted_state.hpp>

namespace quippy {

    struct ConfigImplErrorHalted
    {
        static constexpr auto state_name() { return "ConfigImplErrorHalted"; }

        template< class Fsm, class SourceState, class TargetState >
        void operator()(expects_completion const &event, Fsm &fsm, SourceState &source, TargetState &target)
        {
            event.completion_handler(error::impl_state::halted);
        }

        template< class Fsm >
        void operator()(event_halt const &event, Fsm &fsm, ConnectorImplHaltedState&source, ConnectorImplHaltedState &target)
        {
            source.invoke();
        }
    };

}
