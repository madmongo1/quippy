#include <quippy/connector_impl.hpp>
#include <boost/msm/back/tools.hpp>

namespace quippy {

    template<> std::string invoke_state_names::operator()(connector_impl_back_end& backend) const {
        return state_names(backend);
    }

    std::string state_names(connector_impl_back_end const& p)
    {
        using Stt = connector_impl_back_end::stt;
        using all_states =  msm::back::generate_state_set<Stt>::type;
        char const* state_names[mpl::size<all_states>::value];
        // fill the names of the states defined in the state machine
        mpl::for_each<all_states,boost::msm::wrap<mpl::placeholders::_1> >
            (msm::back::fill_state_names<Stt>(state_names));

        std::string result;

        for (unsigned int i=0;i<connector_impl_back_end::nr_regions::value;++i)
        {
            if (i)
                result += ", ";
            result += state_names[p.current_state()[i]];
        }

        return result;
    }


    auto connector_impl_::container() -> container_type&
    {
        return static_cast<container_type&>(*this);
    }

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

    auto subscribe_halted(connector_impl &impl, connector_impl::halted_slot_type&& slot) -> subscription
    {
        auto &state = impl.get_state<HaltedState&>();
        return state.subscribe(std::move(slot));

    }

}
