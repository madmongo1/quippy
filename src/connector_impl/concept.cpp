//
// Created by Richard Hodges on 25/04/2017.
//

#include <quippy/connector_impl/concept.hpp>

namespace quippy
{


    auto get_shared(connector_impl_concept& concept) -> std::shared_ptr<connector_impl_concept>
    {
        return concept.shared_from_this();
    }

    detail::asio_connection_handler &get_connection_handler(connector_impl_concept &impl)
    {
        return impl.get_connection_handler();
    }

    auto get_socket(connector_impl_concept &impl) -> connector_impl_concept::tcp::socket&
    {
        return impl.get_socket();
    }

    asio::io_service &get_executor(connector_impl_concept &impl) {
        return impl.get_executor();
    }

    template<>
    void process_event(connector_impl_concept& impl, event_connect_tcp const& event)
    {
        impl.get_state_machine().process_event(event);
    }

    template<>
    void process_event(connector_impl_concept& impl, event_connect_protocol const& event)
    {
        impl.get_state_machine().process_event(event);
    }

    template<>
    void process_event(connector_impl_concept& impl, event_halt const& event)
    {
        impl.get_state_machine().process_event(event);
    }

}