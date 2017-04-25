//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/detail/connection_handler.hpp>
#include <quippy/connector_impl/traits.hpp>

namespace quippy
{
    struct connector_impl_concept;

    detail::asio_connection_handler &get_connection_handler(connector_impl_concept &impl);

    asio::ip::tcp::socket &get_socket(connector_impl_concept &impl);

    asio::io_service &get_executor(connector_impl_concept &impl);

    auto get_shared(connector_impl_concept &impl) -> std::shared_ptr<connector_impl_concept>;

    template<typename Event>
    void process_event(connector_impl_concept &impl, Event const &event);

    template<typename Event>
    void notify_event(connector_impl_concept &impl, Event &&evt) {

        auto self = get_shared(impl);

        get_executor(impl).dispatch(
            [self, evt = std::forward<Event>(evt)]() {
                process_event(*self, evt);
            });
    }

    auto subscribe_halted(connector_impl_concept &impl, connector_impl_traits::halted_slot_type &&slot) -> subscription;

}
