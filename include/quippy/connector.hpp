//
// Created by Richard Hodges on 20/04/2017.
//

#pragma once
#include <quippy/config.hpp>
#include <quippy/connector_service.hpp>
#include <quippy/detail/io_object.hpp>

namespace quippy {


    struct connector : detail::io_object<connector_service>
    {
        using base_class = detail::io_object<connector_service>;

        using implementation_class = service_type::implementation_class;

        using tcp = implementation_class::tcp;

        using base_class :: base_class;


        void halt()
        {
            get_service().halt(get_impl());
        }

        template<class Handler>
        void async_connect_link(tcp::resolver::iterator iter, Handler&& handler)
        {
            get_service().async_connect_link(get_impl(), iter, std::forward<Handler>(handler));
        }

        auto connect_link(asio::ip::tcp::resolver::iterator iter, asio::error_code& ec) -> asio::error_code&
        {
            ec.clear();
            get_service().connect_link(get_impl(), iter, ec);
            return ec;
        }

        void connect_link(asio::ip::tcp::resolver::iterator iter) {
            asio::error_code ec;
            get_service().connect_link(get_impl(), iter, ec);
            if (ec) {
                throw asio::system_error(ec, "connect_link: " + ec.message());
            }
        }

        void connect(AMQP::Login const &login, std::string const &vhost, asio::error_code& ec) {
            get_service().connect(get_impl(), login, vhost, ec);
        }

        void connect(AMQP::Login const &login, std::string const &vhost = "/") {
            asio::error_code ec;
            get_service().connect(get_impl(), login, vhost, ec);
            if (ec) {
                throw asio::system_error(ec, "connect: " + ec.message());
            }
        }

        implementation_class &get_impl() { return get_implemenentation(); }
    };
}