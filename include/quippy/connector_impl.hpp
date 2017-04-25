//
// Created by Richard Hodges on 20/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/handlers.hpp>
#include <thread>
#include <mutex>
#include <deque>
#include <amqpcpp.h>
#include <typeinfo>
#include <iostream>
#include <boost/variant.hpp>
#include <quippy/sm.hpp>

#include <quippy/connector_impl/traits.hpp>
#include <quippy/connector_impl/transport_down.hpp>
#include <quippy/connector_impl/transport_resolving.hpp>
#include <quippy/connector_impl/transport_connecting.hpp>
#include <quippy/connector_impl/transport_up.hpp>
#include <quippy/connector_impl/transport_shutting_down.hpp>
#include <quippy/detail/connection_handler.hpp>
#include "error.hpp"

namespace quippy {

    struct event_connect_tcp : expects_completion, connector_impl_traits {
        event_connect_tcp(tcp::resolver::iterator iter, completion_handler_function completion_handler)
            : expects_completion(std::move(completion_handler)),
              iter(std::move(iter)) {}

        tcp::resolver::iterator iter;
    };

    struct event_connect_error {
        asio::error_code error;
    };


    class HaltedState : public base_state<HaltedState> {
        using signal_type = sig::signal<void()>;

    public:
        using slot_type = signal_type::slot_type;
        using base_state::on_entry;
        using base_state::on_exit;

        template<class Event, class FSM>
        void on_entry(Event const & event, FSM & fsm) {
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

    template<class FSM>
    struct state_name {
        using fsm_type = FSM;

        std::string operator()(fsm_type &fsm, std::size_t state) const;
    };

    struct invoke_state_names {
        template<class BackEnd>
        auto operator()(BackEnd &backend) const -> std::string;
    };

    template<class BackEnd>
    struct to_implementation_class {
        using type = BackEnd;
    };

    template<class BackEnd>
    decltype(auto) cast_to_implementation(BackEnd &backend) {
        using impl_class = typename to_implementation_class<BackEnd>::type;
        return static_cast<impl_class &>(backend);
    }

    struct connector_impl;

    struct connector_impl_
        : msm::front::state_machine_def<connector_impl_>,
          std::enable_shared_from_this<connector_impl_>,
          connector_impl_traits {
        using me = connector_impl_;
        using shared_ptr = std::shared_ptr<me>;
        using weak_ptr = std::weak_ptr<me>;

        using halted_slot_type = HaltedState::slot_type;

        using container_type = connector_impl;

        connector_impl_(detail::asio_connection_handler &connection_handler)
            : connection_handler_(connection_handler), socket_(get_executor()) {

        }

        auto container() -> container_type &;

        auto get_shared() -> std::shared_ptr<container_type> {
            return std::shared_ptr<container_type>(this->shared_from_this(), std::addressof(container()));
        }

        using event_connect_tcp = event_connect_tcp;
        using event_connect_protocol = event_connect_protocol;
        using event_halt = event_halt;

        struct event_transport_up {};

        struct TransportDown : base_state<TransportDown>, has_completion_handlers {
            using base_state::on_entry;
            using base_state::on_exit;

        };


        struct TransportConnecting : base_state<TransportConnecting>, has_completion_handlers {
            using base_state::on_entry;
            using base_state::on_exit;

            template<class FSM>
            void on_entry(event_connect_tcp const &event, FSM &fsm) {
                base_state ::on_entry(event, fsm);
                add_completion_handler(event.completion_handler);
                auto lifetime = fsm.shared_from_this();
                asio::async_connect(fsm.socket(), event.iter, [lifetime, &fsm](auto &&ec, auto iter) {
                    if (ec) {
                        fsm.process_event(event_connect_error{ec});
                    } else {
                        fsm.process_event(event_transport_up());
                    }
                });
            }

            template<class Event, class FSM>
            void on_exit(Event const & event, FSM & fsm) {
                base_state ::on_exit(event, fsm);
                QUIPPY_LOG(error) << "invalid exit";
                this->fire_completion_handlers(asio::error_code(asio::error::operation_aborted));
            }

            template<class FSM>
            void on_exit(event_transport_up const &event, FSM &fsm) {
                base_state ::on_exit(event, fsm);
                this->fire_completion_handlers(asio::error_code());
            }

            template<class FSM>
            void on_exit(event_halt const &event, FSM &fsm) {
                base_state ::on_exit(event, fsm);
                this->fire_completion_handlers(asio::error_code(asio::error::operation_aborted));
            }


        };

        using TransportUp_ = connector_impl_transport_up_;
        using TransportUp = connector_impl_transport_up;

        struct SetParent {
            template<class Event, class FSM, class Source>
            void operator()(Event const &event, FSM &fsm, Source &, TransportUp &target) const {
                target.set_parent(fsm);
            }
        };

        struct ErrorHalted {
            template<class Fsm, class SourceState, class TargetState>
            void operator()(expects_completion const &event, Fsm &fsm, SourceState &source, TargetState &target) {
                event.completion_handler(error::impl_state::halted);
            }

            template<class Fsm>
            void operator()(event_halt const &event, Fsm &fsm, HaltedState &source, HaltedState &target) {
                source.invoke();
            }
        };

        struct ErrorDown {
            template<class Fsm, class SourceState, class TargetState>
            void operator()(expects_completion const &event, Fsm &fsm, SourceState &, TargetState &) {
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

        template<typename Event>
        void notify_event(Event &&evt) {
            auto self = shared_from_this();

            this->get_executor().dispatch(
                [this, self, evt = std::forward<Event>(evt)]() {
                    auto &fsm = static_cast<msm::back::state_machine<me> &>(*this);
                    fsm.process_event(evt);
                });
        }

        auto get_connection_handler() const -> detail::asio_connection_handler & {
            return connection_handler_;
        }

        auto get_executor() const -> asio::io_service & {
            return connection_handler_.get_worker_executor();
        }

        auto socket() -> tcp::socket & {
            return socket_;
        }

        template<typename FSM, typename Event>
        void no_transition(Event const &e, FSM &fsm, int state) {

            auto invoker = invoke_state_names();
            auto names = invoker(fsm);

            std::cerr << "connector_impl: no transition from state " << names
                      << " on event " << typeid(e).name() << std::endl;
//            assert(!"improper transition in http state machine");
        }

        detail::asio_connection_handler &connection_handler_;
        tcp::socket socket_;

    };

    using connector_impl_back_end = msm::back::state_machine<connector_impl_>;

    std::string state_names(connector_impl_back_end const &p);

    struct connector_impl;

    template<>
    struct to_implementation_class<connector_impl_back_end> {
        using type = connector_impl;
    };

    struct connector_impl : connector_impl_back_end {
        using back_end =  connector_impl_back_end;

        using back_end::back_end;


    };

    auto subscribe_halted(connector_impl &impl, connector_impl::halted_slot_type &&slot) -> subscription;

/*
struct connector_impl :
    std::enable_shared_from_this<connector_impl>,
    sm::state_machine<connector_impl> {

    using tcp = asio::ip::tcp;
    using resolver = tcp::resolver;


    // events
    struct event_halt { completion_handler_function completion_handler; };
    struct event_connect_tcp {
        tcp::resolver::iterator iter;
        completion_handler_function completion_handler;
    };

    struct event_protocol_connect {
        AMQP::Login login;
        std::string vhost;
        completion_handler_function completion_handler;
    };

    struct event_transport_error { asio::error_code error; };
    struct event_transport_up {};

    // states
    struct transport_down {};
    struct transport_resolving {};
    using transport_connecting = connection_impl_transport_connecting;
    using transport_up = connector_impl_transport_up;
    struct transport_shutting_down {};

    using state_storage_type = state_storage<transport_down,
        transport_resolving,
        transport_connecting,
        transport_up,
        transport_shutting_down>;

    // data

private:
    detail::asio_connection_handler &handler_;
    state_storage_type state_storage_;
    tcp::socket socket_{get_worker_executor()};

public:
    connector_impl(detail::asio_connection_handler &handler)
        : handler_(handler) {

    }

    void notify_stop() {

    }


    /// Notify an event in the current state and handle immediately


    auto get_worker_executor() -> asio::io_service & {
        return handler_.get_worker_executor();
    }

    auto& state_storage() { return state_storage_; }


    /// Valid Event Handlers

    void handle_event(event_connect_tcp event, transport_down &state);

    void handle_event(event_transport_up event, transport_connecting &state);

    void handle_event(event_protocol_connect event, transport_up& state);


    /// handle any unhandled events
    template<class DefaultEvent, class DefaultState>
    void handle_event(DefaultEvent&& event, DefaultState&& state) {
        std::cerr << "UNHANDLED EVENT:\n"
                  << "EVENT: " << notstd::category<decltype(event)>::name() << "\n"
                  << "STATE: " << notstd::category<decltype(state)>::name() << std::endl;
        std::exit(100);

    };
};

template<class Event>
void notify_event(connector_impl &impl, Event&& event) {
    impl.notify_event(std::forward<Event>(event));
}

template<class Event>
void post_event(connector_impl &impl, Event &&event) {
    impl.get_worker_executor().post([self = impl.shared_from_this(), event = std::forward<Event>(event)]() mutable {
        notify_event(*self, std::move(event));
    });
}
 */
}
