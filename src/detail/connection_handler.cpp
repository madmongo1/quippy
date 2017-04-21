//
// Created by Richard Hodges on 21/04/2017.
//

#include <quippy/detail/connection_handler.hpp>
#include <quippy/detail/connection.hpp>

namespace quippy { namespace detail {

    asio_connection_handler::asio_connection_handler(asio::io_service &worker_executor)
        : worker_executor_(worker_executor) {

    }

    auto asio_connection_handler::onData(AMQP::Connection *oconn, const char *buffer, size_t size) -> void {
        connections_.at(oconn)->queue_send(buffer, size);
    }
}}