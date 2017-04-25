//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/sm.hpp>
#include <quippy/connector_impl/concept_fwd.hpp>
#include <quippy/detail/has_environment.hpp>

#include <quippy/event/connect_tcp.hpp>

#include <quippy/connector_impl/states/halted_state.hpp>
#include <quippy/connector_impl/functors/error_halted.hpp>

#include <quippy/connector_impl/transport_up.hpp>
#include <quippy/sm/invoke_state_names.hpp>

namespace quippy {


    struct connector_impl_
    : msm::front::state_machine_def<connector_impl_>
    , detail::has_environment<connector_impl_concept>
    , connector_impl_traits
    {
        using me = connector_impl_;

        using HaltedState = ConnectorImplHaltedState;
        using halted_slot_type = HaltedState::slot_type;

        connector_impl_(environment_type *environment)
        : has_environment(environment)
        {
        }

        using event_connect_tcp = event_connect_tcp;
        using event_halt = event_halt;

        struct event_transport_up {};


        struct TransportDown : base_state<TransportDown>, has_completion_handlers
        {
            using base_state::on_entry;
            using base_state::on_exit;

        };


        struct TransportConnecting : base_state<TransportConnecting>, has_completion_handlers
        {
            using base_state::on_entry;
            using base_state::on_exit;

            template< class FSM >
            void on_entry(event_connect_tcp const &event, FSM &fsm)
            {
                base_state::on_entry(event, fsm);
                add_completion_handler(event.completion_handler);

                auto &env = environment_of(fsm);
                auto lifetime = get_shared(env);
                asio::async_connect(get_socket(env),
                                    event.iter,
                                    [lifetime, &fsm](auto &&ec, auto iter)
                                    {
                                        if (ec) {
                                            fsm.process_event(event_connect_error{ec});
                                        } else {
                                            fsm.process_event(event_transport_up());
                                        }
                                    });
            }

            template< class Event, class FSM >
            void on_exit(Event const &event, FSM &fsm)
            {
                base_state::on_exit(event, fsm);
                QUIPPY_LOG(error) << "invalid exit";
                this->fire_completion_handlers(asio::error_code(asio::error::operation_aborted));
            }

            template< class FSM >
            void on_exit(event_transport_up const &event, FSM &fsm)
            {
                base_state::on_exit(event, fsm);
                this->fire_completion_handlers(asio::error_code());
            }

            template< class FSM >
            void on_exit(event_halt const &event, FSM &fsm)
            {
                base_state::on_exit(event, fsm);
                this->fire_completion_handlers(asio::error_code(asio::error::operation_aborted));
            }


        };

        using TransportUp_ = connector_impl_transport_up_;
        using TransportUp = connector_impl_transport_up;

        struct SetParent
        {
            template< class Event, class FSM, class Source >
            void operator()(Event const &event, FSM &fsm, Source &, TransportUp &target) const
            {
                target.set_environment(environment_of(fsm));
            }
        };

        using ErrorHalted = ConfigImplErrorHalted;

        struct ErrorDown
        {
            template< class Fsm, class SourceState, class TargetState >
            void operator()(expects_completion const &event, Fsm &fsm, SourceState &, TargetState &)
            {
                event.completion_handler(error::impl_state::transport_down);
            }
        };

        using transition_table = mpl::vector<
        msmf::Row < TransportDown, event_connect_tcp, TransportConnecting, msmf::none, msmf::none>,
        msmf::Row <TransportDown, event_connect_protocol, TransportDown, ErrorDown, msmf::none>,
        msmf::Row <TransportDown, event_halt, HaltedState, msmf::none, msmf::none>,

        msmf::Row <TransportConnecting, event_connect_error, TransportDown, msmf::none, msmf::none>,
        msmf::Row <TransportConnecting, event_transport_up, TransportUp, SetParent, msmf::none>,
        msmf::Row <TransportConnecting, event_halt, TransportDown, msmf::none, msmf::none>,

        msmf::Row <TransportUp::exit_pt
        <TransportUp_::AbortExit>, event_abort_complete, HaltedState, msmf::none, msmf::none>,

        msmf::Row <HaltedState, event_halt, msmf::none, ErrorHalted, msmf::none>,
        msmf::Row <HaltedState, event_connect_protocol, msmf::none, ErrorHalted, msmf::none>,
        msmf::Row <HaltedState, event_connect_tcp, msmf::none, ErrorHalted, msmf::none>

        >;



        using initial_state = TransportDown;

        template< typename FSM, typename Event >
        void no_transition(Event const &e, FSM &fsm, int state)
        {

            auto invoker = invoke_state_names();
            auto names = invoker(fsm);

            std::cerr << "connector_impl: no transition from state " << names
                      << " on event " << typeid(e).name() << std::endl;
//            assert(!"improper transition in http state machine");
        }

    };

    using connector_impl_back_end = msm::back::state_machine<connector_impl_>;

    std::string state_names(connector_impl_back_end const &p);

}