//
// Created by Richard Hodges on 21/04/2017.
//

#include <quippy/detail/connection_handler.hpp>

namespace quippy { namespace detail {

    asio_connection_handler::asio_connection_handler(asio::io_service &worker_executor)
        : worker_executor_(worker_executor) {

    }

    auto asio_connection_handler::onData(AMQP::Connection *oconn, const char *buffer, size_t size) -> void {
        std::cerr << __func__ << " size: " << size << std::endl;
        auto &ci = connections_[oconn];

        if (ci.connection.empty()) {
            std::cerr << __func__ << " deferring " << size << std::endl;
            ci.deferred_data.insert(std::end(ci.deferred_data), buffer, buffer + size);
        } else {
            flush_data(ci, buffer, size);
        }
    }

    void asio_connection_handler::flush_data(connection_info &ci, const char *buffer, std::size_t size) {
        assert(not ci.connection.empty());
        if (not ci.deferred_data.empty()) {
            ci.connection.onData(ci.deferred_data.data(), ci.deferred_data.size());
            ci.deferred_data.clear();
        }
        if (buffer) {
            ci.connection.onData(buffer, size);
        }
    }

    void asio_connection_handler::bingo(AMQP::Connection *key, connection_handler_target &&target) {
        assert(key);
        auto &ci = connections_[key];
        assert(ci.connection.empty());
        ci.connection = std::move(target);
        flush_data(ci);
    }

    void asio_connection_handler::unbingo(AMQP::Connection *key) {
        assert(key);
        connections_.erase(key);
    }

    void asio_connection_handler::onConnected(AMQP::Connection *key) {
        std::cerr << __func__ << std::endl;
        connections_.at(key).connection.onConnected();
    }

    void asio_connection_handler::onClosed(AMQP::Connection *key) {
        std::cerr << __func__ << std::endl;
        connections_.at(key).connection.onClosed();
    }

    void asio_connection_handler::onError(AMQP::Connection *key, const char *message) {
        std::cerr << __func__ << " message: " << message << std::endl;
        connections_.at(key).connection.onError(message);
    }


}}