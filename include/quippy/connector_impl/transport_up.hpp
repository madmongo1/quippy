//
// Created by Richard Hodges on 22/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/sm.hpp>
#include <quippy/handlers.hpp>
#include <quippy/detail/connection_handler.hpp>
#include <quippy/connector_impl/traits.hpp>
#include <quippy/sm.hpp>
#include <quippy/sm/default_transition.hpp>
#include <amqpcpp.h>
#include <boost/optional.hpp>

#include <quippy/connector_impl/concept_fwd.hpp>
#include <quippy/detail/has_environment.hpp>
#include <quippy/detail/send_buffer.hpp>

#include <quippy/event/halt.hpp>
#include <quippy/event/connect_protocol.hpp>

namespace quippy {

    struct event_send_data {
        event_send_data(const char *buffer, std::size_t size) :
            buffer(buffer), size(size) {
        }

        const char *buffer;
        std::size_t size;
    };


    struct event_protocol_connected {};
    struct event_protocol_closed {};

    struct event_protocol_error {
        event_protocol_error(asio::error_code ec) : error(ec) {}

        asio::error_code error;
    };

    struct event_send_error {
        event_send_error(asio::error_code ec) : error(ec) {}

        asio::error_code error;
    };

    struct event_receive_error {
        event_receive_error(asio::error_code ec) : error(ec) {}

        asio::error_code error;
    };

    struct connector_impl_;

    struct WaitingDeath : quippy::base_state<WaitingDeath> {
        using base_state::on_entry;
        using base_state::on_exit;

        template<class Event, class FSM>
        void on_entry(Event &event, FSM &fsm) {
            std::cerr << "WaitingDeath: " << typeid(Event).name() << std::endl;
            assert(initial_error);
        }

        template<class FSM>
        void on_entry(event_halt const &event, FSM &fsm) {
            initial_error = asio::error::operation_aborted;
        }


        asio::error_code initial_error;
    };

    struct WaitingExitTransportDown : quippy::base_state<WaitingExitTransportDown> {
        using base_state::on_entry;
        using base_state::on_exit;

        template<class Event, class FSM>
        void on_entry(Event &event, FSM &fsm) {
            base_state::on_entry(event, fsm);
            assert(initial_error);
        }

        template<class FSM>
        void on_entry(event_send_error const &event, FSM &fsm) {
            base_state::on_entry(event, fsm);
            initial_error = event.error;
        }

        template<class FSM>
        void on_entry(event_receive_error const &event, FSM &fsm) {
            base_state::on_entry(event, fsm);
            initial_error = event.error;
        }


        asio::error_code initial_error;
    };


    struct event_abort_complete {
        event_abort_complete() {}

        template<class Arg>
        event_abort_complete(Arg const&) {}
    };

    struct transport_up_invoker {
        template<class FSM>
        void invoke_maybe_send(FSM &fsm) const;

        template<class FSM>
        void invoke_initiate_connect(FSM &fsm, AMQP::Login const &login, std::string const &vhost) const;
    };


    struct connector_impl_transport_up_
        : detail::has_environment<connector_impl_concept>,
          msmf::state_machine_def<connector_impl_transport_up_,
            base_state<connector_impl_transport_up_>>,
          connector_impl_traits {

        using base_state::on_entry;
        using base_state::on_exit;

        using event_connect_protocol = event_connect_protocol;

        struct NotConnected : quippy::base_state<NotConnected> {
            using base_state::on_entry;
            using base_state::on_exit;
        };

        struct Connecting : quippy::base_state<Connecting>, has_completion_handlers {
            using base_state::on_entry;
            using base_state::on_exit;

            template<class Event, class FSM>
            void on_entry(Event const &event, FSM &fsm) {
                base_state::on_entry(event, fsm);
                assert(!"invalid event");
            }

            template<class FSM>
            void on_entry(event_connect_protocol const &event, FSM &fsm) {
                base_state::on_entry(event, fsm);
                add_completion_handler(event.completion_handler);
                transport_up_invoker invoker;
                invoker.invoke_initiate_connect(fsm, event.login, event.vhost);
            }

            template<class Event, class FSM>
            void on_exit(Event const &event, FSM &fsm) {
                base_state::on_exit(event, fsm);
                assert(!"invalid event");
            };

            template<class FSM>
            void on_exit(event_protocol_connected const &event, FSM &fsm) {
                base_state::on_exit(event, fsm);
                this->fire_completion_handlers(asio::error_code());
            }

            template<class FSM>
            void on_exit(event_protocol_error const &event, FSM &fsm) {
                base_state::on_exit(event, fsm);
                this->fire_completion_handlers(event.error);
            }

            template<class FSM>
            void on_exit(event_halt const &event, FSM &fsm) {
                base_state::on_exit(event, fsm);
                this->fire_completion_handlers(asio::error::operation_aborted);
            }

            template<class FSM>
                void on_exit(event_send_error const& event, FSM& fsm)
            {
                base_state::on_exit(event, fsm);
                fsm.unbingo(event.error);
            }

            template<class FSM>
            void on_exit(event_receive_error const& event, FSM& fsm)
            {
                base_state::on_exit(event, fsm);
                fsm.unbingo(event.error);
            }
        };

        struct Connected : quippy::base_state<Connected> {
            using base_state::on_entry;
            using base_state::on_exit;

            template<class FSM>
            void on_exit(event_halt const& event, FSM& fsm)
            {
                base_state::on_exit(event, fsm);
                fsm.unbingo(asio::error::operation_aborted);
            }
        };
        struct ErrorState : msmf::state<> {};

        struct AbortExit : msmf::exit_pseudo_state<event_abort_complete> {
            using base_state = quippy::base_state<AbortExit>;
            base_state base;

            template<class Event, class FSM>
            void on_entry(Event const& event, FSM& fsm) {
                base.on_entry(event, fsm);
            };

            template<class Event, class FSM>
            void on_exit(Event const& event, FSM& fsm) {
                base.on_exit(event, fsm);
            };

         };


        using initial_state = NotConnected;

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

                fsm.unbingo(asio::error::operation_aborted);
            }

            template<class Fsm, class Target>
            void operator()(asio::error_code const &event, Fsm &fsm, Connecting &source, Target &target) {

                fsm.unbingo(event);
                source.fire_completion_handlers(event);
            }

        };

        struct OnClose {
            template<class Event, class Fsm, class Source, class Target>
            void operator()(Event const &, Fsm &fsm, Source &, Target &) {
                std::cout << "connector_impl_transport_up::OnClose" << std::endl;
                fsm.close();
            }

        };

        struct PostError {
            template<class Event, class Fsm, class Source, class Target>
            bool operator()(Event const &event, Fsm &fsm, Source &, Target &) {
                return fsm.get_environment().get_state_machine().process_event(event.error);
            }
        };

        struct IsCommsError {
            template<class Event, class Fsm, class Source, class Target>
            bool operator()(Event const &event, Fsm &fsm, Source &, Target &) {
                return event.error;
            }
        };

        struct Complete {
            template<class Event, class Fsm, class Source, class Target>
            bool operator()(Event const &event, Fsm &fsm, Source &source, Target &) {
            }
        };


        struct HandleDataFlow {
            template<class FSM, class Source, class Target>
            void operator()(event_send_data const &event, FSM &fsm, Source &, Target &) {
                transport_up_invoker invoker;
                fsm.get_send_state().put_data(event.buffer, event.size);
                invoker.invoke_maybe_send(fsm);
            }

        };

        struct IsNoIoOutstanding {
            template<class Event, class Fsm, class Source, class Target>
            bool operator()(Event const &, Fsm &fsm, Source &source, Target &) {
                return not fsm.get_send_state().sending()
                       and not fsm.get_receive_state().receiving()
                       and not fsm.bingoed();
            }
        };


        struct transition_table
            : mpl::vector<
                msmf::Row < NotConnected, event_halt, AbortExit, msmf::none, msmf::none>,
              msmf::Row<NotConnected, event_connect_protocol, Connecting, msmf::none, msmf::none>,

              msmf::Row<Connecting, event_send_data, msmf::none, HandleDataFlow, msmf::none>,
              msmf::Row<Connecting, event_halt, WaitingDeath, msmf::none, msmf::none>,
              msmf::Row<Connecting, event_send_error, WaitingExitTransportDown, msmf::none, msmf::none>,
              msmf::Row<Connecting, event_receive_error, WaitingExitTransportDown, msmf::none, msmf::none>,
            msmf::Row<Connecting, event_protocol_connected, Connected, msmf::none, msmf::none>,
            msmf::Row<Connecting, event_protocol_error, NotConnected, msmf::none, msmf::none>,

              msmf::Row<Connected, event_halt, WaitingDeath, msmf::none, msmf::none>,



              msmf::Row<WaitingDeath, event_receive_error, AbortExit, msmf::none, IsNoIoOutstanding>,
              msmf::Row<WaitingDeath, event_send_error, AbortExit, msmf::none, IsNoIoOutstanding>,
              msmf::Row<WaitingDeath, event_protocol_error, AbortExit, msmf::none, IsNoIoOutstanding>
        > {};


        template<typename FSM, typename Event>
        void no_transition(Event const &e, FSM &, int state) {
            std::cerr << "transport_up: no transition from state " << state
                      << " on event " << typeid(e).name() << std::endl;
//            assert(!"improper transition in http state machine");
        }


        auto get_connection_ptr() {
            return connection_.get_ptr();
        }

        void bingo(const AMQP::Login &login, const std::string &vhost, detail::connection_handler_target &&target);

        void cancel_io();

        void unbingo(asio::error_code const &error);

        void unbingo();


        void handle_send(asio::error_code const &ec, std::size_t size);

        void handle_receive(asio::error_code const &ec, std::size_t size);

        bool bingoed() const { return connection_.is_initialized(); }


        auto get_protocol_connection() -> AMQP::Connection & {
            return connection_.get();
        }


        struct receive_state {

            bool receiving() const { return receiving_; }

            auto prepare() -> asio::streambuf::mutable_buffers_type {
                assert(not receiving());
                receiving_ = true;
                return receive_buffer_.prepare(4096);
            }

            void commit(std::size_t bytes) {
                assert(receiving());
                receiving_ = false;
                receive_buffer_.commit(bytes);
            }

            struct run_info {
                operator bool() const { return length > 0; }

                const char *const data;
                std::size_t const length;
            };

            auto get_at_least(std::size_t required) const -> run_info {
                assert(not receiving());
                auto buffer = receive_buffer_.data();
                auto available = asio::buffer_size(buffer);
                if (available >= required) {
                    return run_info {asio::buffer_cast<const char *>(buffer), available};
                } else {
                    return run_info {nullptr, 0};
                }
            }

            void consume(std::size_t bytes) {
                assert(not receiving());
                assert(bytes <= asio::buffer_size(receive_buffer_.data()));
                receive_buffer_.consume(bytes);
            }

            asio::streambuf receive_buffer_;
            bool receiving_ = false;
        };

        auto get_send_state() -> detail::send_buffer & {
            return send_state_;
        }

        auto get_receive_state() -> receive_state & {
            return receive_state_;
        }

        auto socket() -> tcp::socket &;

    private:
        connector_impl_ *parent_ = nullptr;
        boost::optional<AMQP::Connection> connection_;
        detail::send_buffer send_state_;
        receive_state receive_state_;
    };

    using connector_impl_transport_up = msm::back::state_machine<connector_impl_transport_up_>;


}