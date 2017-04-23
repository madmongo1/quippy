//
// Created by Richard Hodges on 22/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/handlers.hpp>
#include <quippy/connector_impl/traits.hpp>
#include <quippy/sm.hpp>
#include <quippy/sm/default_transition.hpp>
#include <amqpcpp.h>
#include <boost/optional.hpp>

namespace quippy {

    struct event_send_data {
        event_send_data(const char *buffer, std::size_t size) :
            data(buffer, buffer + size) {

        }

        std::vector<char> data;
    };

    struct event_connect_protocol : connector_impl_traits {
        event_connect_protocol(AMQP::Login const &login,
                               std::string const &vhost,
                               completion_handler_function &&f)
            : login(std::move(login)),
              vhost(std::move(vhost)),
              completion_handler(std::move(f)) {}

        AMQP::Login login;
        std::string vhost;
        completion_handler_function completion_handler;
    };

    struct event_protocol_connected {};
    struct event_protocol_closed {};
    struct event_protocol_error {
        template<class String>
        event_protocol_error(String&& str) : message(std::forward<String>(str)) {}

        std::string message;
    };

    struct event_received_data {
        event_received_data(asio::error_code const& ec) : error(ec) {}

        asio::error_code error;
    };

    struct connector_impl_;

    struct connector_impl_transport_up_ : msmf::state_machine_def<connector_impl_transport_up_> {
        void set_parent(connector_impl_ &parent) {
            parent_ = std::addressof(parent);
        }

        using event_connect_protocol = event_connect_protocol;

        struct NotConnected : msmf::state<> {};
        struct Connecting : msmf::state<>, has_completion_handlers {};
        struct Connected : msmf::state<> {};
        struct ErrorState : msmf::state<> {};

        struct ExitWithCommsError : msmf::exit_pseudo_state<asio::error_code> {};
        struct ExitWithHalt : msmf::exit_pseudo_state<event_halt> {};

        struct WaitingDeath : msmf::state<>, has_completion_handlers {};

        using initial_state = NotConnected;

        void initiate_connect(const AMQP::Login& login, const std::string& vhost);

        struct InitiateConnect : DefaultTransition {
            template<class Fsm>
            void operator()(event_connect_protocol const &event, Fsm &fsm, NotConnected &source, Connecting &target) {
                target.add_completion_handler(std::move(event.completion_handler));
                fsm.initiate_connect(event.login, event.vhost);
            }
        };

        struct SendData : DefaultTransition {
            template<class Fsm, class Source, class Target>
            void operator()(event_send_data const &event, Fsm &fsm, Source &source, Target &target) {
                fsm.add_send_data(event.data);
            }


        };

        struct HandleReceivedData : DefaultTransition {
            template<class Event, class Fsm, class Source, class Target>
            void operator()(Event const &event, Fsm &fsm, Source &source, Target &target) {
                fsm.do_receive_processing(event);
            }

            using DefaultTransition::operator();

        };

        struct OnProtocolConnect : DefaultTransition {
            using DefaultTransition::operator();
            template<class Fsm, class Target>
            void operator()(event_protocol_connected const &event, Fsm &fsm, Connecting &source, Target &target) {
                source.fire_completion_handlers(asio::error_code());
            }

        };

        struct ForceShutdown {
            template<class Event, class Fsm, class Source, class Target>
            void operator()(Event const &event, Fsm &fsm, Source &, Target &target) {

                target.add_completion_handler(event.handler);
                fsm.unbingo(asio::error::operation_aborted);
            }

            template<class Fsm, class Target>
            void operator()(asio::error_code const &event, Fsm &fsm, Connecting &source, Target &target) {

                fsm.unbingo(event);
                source.fire_completion_handlers(event);
            }

        };

        struct OnClose
        {
            template<class Event, class Fsm, class Source, class Target>
            void operator()(Event const &, Fsm &fsm, Source &, Target &) {
                std::cout << "connector_impl_transport_up::OnClose" << std::endl;
                fsm.close();
            }

        };

        struct PostError
        {
            template<class Event, class Fsm, class Source, class Target>
            bool operator()(Event const &event, Fsm &fsm, Source &, Target &) {
                return fsm.process_event(event.error);
            }
        };

        struct IsCommsError
        {
            template<class Event, class Fsm, class Source, class Target>
            bool operator()(Event const &event, Fsm &fsm, Source &, Target &) {
                return event.error;
            }
        };

        struct Complete
        {
            template<class Event, class Fsm, class Source, class Target>
            bool operator()(Event const &event, Fsm &fsm, Source & source, Target &) {
                source.fire_completion_handlers(event);
            }
        };

        struct transition_table
            : mpl::vector<
                msmf::Row < NotConnected, event_connect_protocol, Connecting, InitiateConnect, msmf::none>,
              msmf::Row < NotConnected, event_halt, ExitWithHalt, msmf::none, msmf::none>,

              msmf::Row<Connecting, event_send_data, msmf::none, SendData, msmf::none>,
              msmf::Row<Connecting, asio::error_code, ExitWithCommsError, ForceShutdown, msmf::none>,
              msmf::Row<Connecting, event_received_data, msmf::none, HandleReceivedData, msmf::none>,
              msmf::Row<Connecting, event_protocol_connected, Connected, OnProtocolConnect, msmf::none>,
              msmf::Row<Connecting, event_halt, WaitingDeath, ForceShutdown, msmf::none>,

              msmf::Row<Connected, event_halt, WaitingDeath, ForceShutdown, msmf::none>,

              msmf::Row<WaitingDeath, event_send_data, msmf::none, DefaultTransition, msmf::none>,
              msmf::Row<WaitingDeath, event_received_data, msmf::none, PostError, IsCommsError>,
              msmf::Row<WaitingDeath, asio::error_code, ExitWithCommsError, Complete, msmf::none>,
              msmf::Row<WaitingDeath, event_protocol_error, msmf::none, DefaultTransition, msmf::none>

        > {};


        template<typename FSM, typename Event>
        void no_transition(Event const &e, FSM &, int state) {
            std::cerr << "transport_up: no transition from state " << state
                      << " on event " << typeid(e).name() << std::endl;
//            assert(!"improper transition in http state machine");
        }


        connector_impl_ *get_parent() const {
            assert(parent_);
            return parent_;
        }

        auto get_connection_ptr() {
            return connection_.get_ptr();
        }

        void cancel_io();
        void unbingo(asio::error_code const& error);
        void unbingo();

        void start_receiving();

        void do_receive_processing(event_received_data const &event);

        void add_send_data(std::vector<char> const &source);

        void maybe_send();
        void handle_send(asio::error_code const& ec, std::size_t size);

    private:
        connector_impl_ *parent_ = nullptr;
        boost::optional<AMQP::Connection> connection_;

        std::vector<char> send_buffer_;
        std::vector<char> sending_buffer_;

        asio::streambuf receive_buffer_;

    };

    using connector_impl_transport_up = msm::back::state_machine<connector_impl_transport_up_>;
}