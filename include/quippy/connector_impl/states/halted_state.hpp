//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once
#include <quippy/config.hpp>
#include <quippy/sm.hpp>

namespace quippy {

    class ConnectorImplHaltedState : public base_state<ConnectorImplHaltedState> {
        using signal_type = sig::signal<void()>;

    public:
        using slot_type = signal_type::slot_type;
        using base_state::on_entry;
        using base_state::on_exit;

        template<class Event, class FSM>
        void on_entry(Event const &event, FSM &fsm) {
            base_state::on_entry(event, fsm);
            invoke();
        }

        auto subscribe(slot_type &&slot) -> sig::connection {
            return signal_.connect(std::move(slot));
        }

        void invoke() {
            signal_();
        }

    private:
        signal_type signal_;
    };

}