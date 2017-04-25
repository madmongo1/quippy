//
// Created by Richard Hodges on 22/04/2017.
//

#include <quippy/connector_impl/transport_up.hpp>
#include <quippy/connector_impl.hpp>

namespace quippy
{
    namespace {


        auto make_connection_handler_target(connector_impl_transport_up &fsm) -> detail::connection_handler_target {
            using fsm_type = connector_impl_transport_up;

            struct model : detail::connection_handler_target::concept {
                model(fsm_type &fsm) : fsm_(fsm) {}

                void onData(const char *buffer, size_t size) override {
                    fsm_.process_event(event_send_data(buffer, size));
                }

                virtual void onConnected() override {
                    fsm_.process_event(event_protocol_connected());
                }

                virtual void onClosed() override {
                    fsm_.process_event(event_protocol_closed());
                }

                virtual void onError(const char *message) override {
                    auto &&category = error::transport_category();
                    auto code = category.get_code(message);
                    fsm_.process_event(event_protocol_error(asio::error_code(code,
                                                                             category)));
                }

                fsm_type &fsm_;
            };

            return detail::connection_handler_target(std::make_unique<model>(fsm));
        }

        void maybe_send(connector_impl_transport_up& fsm);

        auto make_send_completion_handler(connector_impl_transport_up& fsm)
        {
            auto lifetime = fsm.get_parent()->get_shared();
            return [lifetime, &fsm](auto&& ec, auto size) {
                fsm.get_send_state().consume(size);
                if (ec) {
                    fsm.process_event(event_send_error(ec));
                }
                else {
                    maybe_send(fsm);
                }
            };
        }

        void maybe_send(connector_impl_transport_up& fsm)
        {
            auto& send_state = fsm.get_send_state();
            if (not send_state.sending() and send_state.has_pending_data()) {
                auto buffer = send_state.prepare();
                asio::async_write(fsm.socket(), buffer, make_send_completion_handler(fsm));
            }
        }

        void start_receiving(connector_impl_transport_up& fsm);

        auto make_receive_completion_handler(connector_impl_transport_up& fsm)
        {
            auto lifetime = fsm.get_parent()->get_shared();
            return [lifetime, &fsm](auto&& ec, auto size)
            {
                auto& rx_state = fsm.get_receive_state();
                rx_state.commit(size);
                if (fsm.bingoed()) {
                    auto &protocol_connection = fsm.get_protocol_connection();
                    while (auto run = rx_state.get_at_least(protocol_connection.expected())) {
                        auto &protocol = fsm.get_protocol_connection();
                        auto consumed = protocol.parse(run.data, run.length);
                        rx_state.consume(consumed);
                    }
                    if (not ec) {
                        return start_receiving(fsm);
                    }
                }

                if (ec) {
                    return void(fsm.process_event(event_receive_error(ec)));
                }
            };

        }

        void start_receiving(connector_impl_transport_up& fsm)
        {
            auto& receive_state = fsm.get_receive_state();
            assert(not receive_state.receiving());
            fsm.socket().async_read_some(receive_state.prepare(), make_receive_completion_handler(fsm));
        }
    }

    template<>
    void transport_up_invoker::invoke_maybe_send(connector_impl_transport_up& fsm) const
    {
        maybe_send(fsm);
    }

    template<>
    void transport_up_invoker::invoke_initiate_connect(connector_impl_transport_up& fsm,
                                                       AMQP::Login const& login,
                                                       std::string const& vhost) const
    {
        assert(not fsm.bingoed());
        fsm.bingo(login, vhost, make_connection_handler_target(fsm));
        start_receiving(fsm);
    }


    void connector_impl_transport_up_::bingo(const AMQP::Login& login,
                                             const std::string& vhost,
                                             detail::connection_handler_target&& target)
    {
        assert(not bingoed());
        auto&& handler = get_parent()->get_connection_handler();
        connection_.emplace(std::addressof(handler), login, vhost);
        handler.bingo(connection_.get_ptr(), std::move(target));
    }


    void connector_impl_transport_up_::cancel_io()
    {
        auto& socket = get_parent()->socket();
        asio::error_code sink;
        socket.cancel(sink);
    }

    void connector_impl_transport_up_::unbingo(asio::error_code const& error)
    {
        QUIPPY_LOG(debug) << "TransportUp::" << __func__ << " : " << error.message();
        auto parent = get_parent();
        auto& handler = parent->get_connection_handler();
        assert(bool(connection_));
        cancel_io();
        auto& connection = connection_.get();
        connection.notifyTransportError("transport error: " + error.message());
        handler.unbingo(connection_.get_ptr());
        connection_.reset();
    }

    void connector_impl_transport_up_::unbingo()
    {
        auto parent = get_parent();
        auto& handler = parent->get_connection_handler();
        assert(bool(connection_));
        auto& connection = connection_.get();
        connection.close();
        handler.unbingo(connection_.get_ptr());
        connection_.reset();
    }

    auto connector_impl_transport_up_::socket() -> tcp::socket & {
        return get_parent()->socket();
    }



}