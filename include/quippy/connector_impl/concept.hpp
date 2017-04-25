//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <quippy/detail/connection_handler.hpp>
#include <quippy/connector_impl/concept_fwd.hpp>
#include <quippy/connector_impl/connector_impl_.hpp>

namespace quippy {

    struct connector_impl_concept
        : std::enable_shared_from_this<connector_impl_concept>
        , connector_impl_traits
    {
        using back_end = msm::back::state_machine<connector_impl_>;

        using HaltedState = back_end::HaltedState;

        connector_impl_concept(detail::asio_connection_handler &handler)
            : connection_handler_(handler)
            , socket_(get_executor())
            , state_machine_(this) {}

        void start()
        {
            state_machine_.start();
        }

        detail::asio_connection_handler& get_connection_handler() const { return connection_handler_; }

        auto get_executor() const -> asio::io_service& { return get_connection_handler().get_worker_executor(); }

        auto get_socket() -> tcp::socket& { return socket_; }

        back_end& get_state_machine() {
            return state_machine_;
        }

    private:

        detail::asio_connection_handler& connection_handler_;
        tcp::socket socket_;
        back_end state_machine_;
    };

}