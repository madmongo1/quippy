//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once
#include <quippy/config.hpp>
#include <quippy/sm.hpp>

namespace quippy
{
    template<class Event>
    struct event_base
    {

    };

    template<class Event>
    struct invoke_event_name
    {
        template<class Arg>
        const char* get_event_name(Arg const& event) const
        {
            return event_name(event);
        }
    };

    template<class FSM>
    struct invoke_event_name<boost::any>
    {
        template<class Arg>
        const char* get_event_name(Arg const& event) const
        {
            return "any";
        }
    };

}