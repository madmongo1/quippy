//
// Created by Richard Hodges on 20/04/2017.
//

#include <quippy/config.hpp>
#include <quippy/connector_service.hpp>


namespace quippy {

    struct connector {
        using service_type = connector_service;
        using implementation_type = service_type::implementation_type;
        using implementation_class = service_type::implementation_class;

        using tcp = implementation_class::tcp;

        connector(asio::io_service &ios)
            : service_ptr_(std::addressof(asio::use_service<service_type>(ios))), impl_(get_service().create()) {}

        connector(connector&&) = default;
        connector& operator=(connector&&) = default;
        ~connector()
        {
            get_service().destroy(get_implementation());
        }

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

        service_type &get_service() const { return *service_ptr_; }

        implementation_type &get_implementation() { return impl_; }
        implementation_class &get_impl() { return *impl_; }

        implementation_type const &get_implementation() const { return impl_; }

    private:
        service_type *service_ptr_;
        implementation_type impl_;
    };
}