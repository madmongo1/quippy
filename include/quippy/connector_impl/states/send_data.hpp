//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/sm.hpp>
#include <quippy/detail/has_environment.hpp>
#include <quippy/detal/send_buffer.hpp>

namespace quippy
{
    struct NotSending : msmf::state<> {};
    struct Sending : msmf::state<> {};
    struct Error : msmf::exit_pseudo_state<event_send_error>;

    template<class Environment>
    struct SendDataState_ : msmf::state_machine_def<SendDataState_>,
    detail::has_environment<Environment>
    {
        template<class Event, class FSM>
        void on_entry(Event&, FSM& fsm)
        {
            this->set_environment(environment_of(fsm));
        }

        using transition_table = mpl::vector<

        >;

        using initial_state = NotSending;



    };

    template<class Environment>
    using SendDataState = msm::back::state_machine<SendDataState_<Environment>>;

}