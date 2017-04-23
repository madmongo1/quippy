//
// Created by Richard Hodges on 21/04/2017.
//

#include <quippy/detail/connection_handler.hpp>
#include <quippy/connector_impl.hpp>
#include <iostream>

namespace quippy { namespace detail {

    asio_connection_handler::asio_connection_handler(asio::io_service &worker_executor)
        : worker_executor_(worker_executor) {

    }

    auto asio_connection_handler::onData(AMQP::Connection *oconn, const char *buffer, size_t size) -> void {
        std::cerr << __func__ << " size: " << size << std::endl;
        auto& ci = connections_[oconn];

        if (ci.connection == nullptr)
        {
            std::cerr << __func__ << " deferring " << size << std::endl;
            ci.deferred_data.insert(std::end(ci.deferred_data), buffer, buffer + size);
        }
        else {
            flush_data(ci, buffer, size);
        }
    }

    void asio_connection_handler::flush_data(connection_info& ci, const char* buffer, std::size_t size)
    {
        assert(ci.connection);
        if (not ci.deferred_data.empty())
        {
            ci.connection->notify_event(event_send_data(ci.deferred_data.data(), ci.deferred_data.size()));
            ci.deferred_data.clear();
        }
        if (buffer) {
            ci.connection->notify_event(event_send_data(buffer, size));
        }
    }

    void asio_connection_handler::bingo(AMQP::Connection* key, connector_impl_* connector)
    {
        assert(key);
        assert(connector);
            auto &ci = connections_[key];
            assert(not ci.connection);
            ci.connection = connector;
            flush_data(ci);
    }

    void asio_connection_handler::unbingo(AMQP::Connection* connection)
    {
        assert(connection);
        connections_.erase(connection);
    }

    void asio_connection_handler::onConnected(AMQP::Connection *connection)
    {
        std::cerr << __func__ << std::endl;
        auto& ci = connections_[connection];
        assert(ci.connection);
        ci.connection->notify_event(event_protocol_connected());
    }

    void asio_connection_handler::onClosed(AMQP::Connection *connection)
    {
        std::cerr << __func__ << std::endl;
        auto& ci = connections_[connection];
        assert(ci.connection);
        ci.connection->notify_event(event_protocol_closed());
    }

    void asio_connection_handler::onError(AMQP::Connection *connection, const char* message)
    {
        std::cerr << __func__ << " message: " << message << std::endl;
        auto& ci = connections_[connection];
        assert(ci.connection);
        ci.connection->notify_event(event_protocol_error(message));

    }


}}