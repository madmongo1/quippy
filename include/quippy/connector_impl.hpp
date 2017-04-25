//
// Created by Richard Hodges on 20/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/connector_impl/concept_fwd.hpp>

namespace quippy {




    template<class FSM>
    struct state_name {
        using fsm_type = FSM;

        std::string operator()(fsm_type &fsm, std::size_t state) const;
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


struct connector_impl : connector_impl_traits
{
    using concept = connector_impl_concept;


    connector_impl(detail::asio_connection_handler& handler);


    void start();

    subscription subscribe_halted(halted_slot_type&& slot);

    void notify_halt();

    template<class Event>
    void post(Event&& event) {
        notify_event(*concept_, std::forward<Event>(event));
    }


    void reset();

private:

    std::shared_ptr<concept> concept_;
};




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
