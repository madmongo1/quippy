

#include <quippy/connector_impl.hpp>

namespace quippy {
/*
    void connector_impl::handle_event(event_connect_tcp event, transport_down& state)
    {
        auto&& destination_state =  state_storage().get(state_tag<transport_connecting>());

        destination_state.add_completion_handler(std::move(event.completion_handler));

        asio::async_connect(socket_,
                            event.iter,
        [self = shared_from_this()](asio::error_code const& ec, auto iter)
        {
            if (ec) {
                self->notify_event(event_transport_error { ec });
            }
            else {
                self->notify_event(event_transport_up {});
            }
        });

        state_storage().set_state(state_tag<transport_connecting>());
    }

    void connector_impl::handle_event(event_transport_up, transport_connecting& state)
    {
        state.dispatch_completion_handlers(asio::error_code());
        auto& target = state_storage().get(state_tag<transport_up>());
        state_storage().set_state(state_tag<transport_up>());
        target.notify_event(event_start(), *this);
    }

    void connector_impl::handle_event(event_protocol_connect event, transport_up& state)
    {

    }
*/

}