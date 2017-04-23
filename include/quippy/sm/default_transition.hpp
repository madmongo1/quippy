//
// Created by Richard Hodges on 22/04/2017.
//

#pragma once

#include <quippy/sm.hpp>
#include <iostream>
#include <typeinfo>
#include <cstdlib>

namespace quippy {
    struct DefaultTransition {
        virtual ~DefaultTransition() {}
        template<class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const &event, Fsm &fsm, SourceState &source, TargetState &target) {
            std::cerr << "unhandled transition:\n";
            std::cerr << "COMMAND: " << typeid(*this).name() << '\n';
            std::cerr << "EVENT  : " << typeid(event).name() << '\n';
            std::cerr << "FSM    : " << typeid(fsm).name() << '\n';
            std::cerr << "SOURCE : " << typeid(source).name() << '\n';
            std::cerr << "TARGET : " << typeid(target).name() << '\n';
        }

    };

}