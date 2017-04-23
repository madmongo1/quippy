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

namespace quippy {

    struct event_connect_tcp : connector_impl_traits {
        event_connect_tcp(tcp::resolver::iterator iter, completion_handler_function completion_handler)
            : iter(std::move(iter)), completion_handler(std::move(completion_handler)) {}

        tcp::resolver::iterator iter;
        completion_handler_function completion_handler;
    };




    struct connector_impl_
        : msm::front::state_machine_def<connector_impl_>,
          std::enable_shared_from_this<connector_impl_>,
          connector_impl_traits {
        using me = connector_impl_;
        using shared_ptr = std::shared_ptr<me>;
        using weak_ptr = std::weak_ptr<me>;

        connector_impl_(detail::asio_connection_handler &connection_handler)
            : connection_handler_(connection_handler)
        , socket_(get_executor())
        {

        }

        auto get_shared() { return this->shared_from_this(); }

        using event_connect_tcp = event_connect_tcp;
        using event_connect_protocol = event_connect_protocol;
        using event_halt = event_halt;

        struct event_transport_up {};

        struct TransportDown : msmf::state<>, has_completion_handlers {};


        struct TransportConnecting : msmf::state<> , has_completion_handlers {

        };

        using TransportUp = connector_impl_transport_up;


        struct StartConnect : DefaultTransition {
            template<class Event, class Fsm, class SourceState, class TargetState>
            void operator()(Event const &event, Fsm &fsm, SourceState &source, TargetState &target) {
                target.add_completion_handler(std::move(event.completion_handler));
                auto self = fsm.shared_from_this();
                asio::async_connect(fsm.socket(), event.iter, [self](auto &&ec, auto iter) {
                    if (ec) {
                        self->notify_event(ec);
                    } else {
                        self->notify_event(event_transport_up());
                    }
                });
                std::cout << "StartConnect" << std::endl;
            }
        };

        struct CompleteTransport : DefaultTransition {
            template<class Fsm, class SourceState, class TargetState>
            void operator()(asio::error_code const &event, Fsm &, SourceState &source, TargetState &) {
                source.fire_completion_handlers(event);
            }

            template<class Fsm, class SourceState, class TargetState>
            void operator()(event_transport_up const &event, Fsm &fsm, SourceState &source, TargetState &target) {
                source.fire_completion_handlers(asio::error_code());
                target.set_parent(fsm);
            }
        };

        struct OnHalt
        {
            template<class Fsm, class SourceState, class TargetState>
            void operator()(event_halt const &event, Fsm &fsm, SourceState &source, TargetState &target) {
                std::cout << "connector_impl::OnHalt" << std::endl;
                event.fire_completion_handler();
            }

        };

        using transition_table = mpl::vector<
            msmf::Row < TransportDown, event_connect_tcp, TransportConnecting, StartConnect, msmf::none>,

        msmf::Row <TransportConnecting, asio::error_code, TransportDown, CompleteTransport, msmf::none>,
        msmf::Row <TransportConnecting, event_transport_up, TransportUp, CompleteTransport, msmf::none>,
        msmf::Row <TransportConnecting, event_halt, TransportDown, OnHalt, msmf::none >,

        msmf::Row <TransportUp, asio::error_code, TransportDown, msmf::none, msmf::none >

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

        auto get_connection_handler() const -> detail::asio_connection_handler&
        {
            return connection_handler_;
        }

        auto get_executor() const -> asio::io_service & {
            return connection_handler_.get_worker_executor();
        }

        auto socket() -> tcp::socket & {
            return socket_;
        }

        template <typename FSM, typename Event>
        void no_transition(Event const& e, FSM&, int state)
        {
            std::cerr << "connector_impl: no transition from state " << state
                      << " on event " << typeid(e).name() << std::endl;
//            assert(!"improper transition in http state machine");
        }

        detail::asio_connection_handler &connection_handler_;
        tcp::socket socket_;

    };

    using connector_impl = msm::back::state_machine<connector_impl_>;

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
