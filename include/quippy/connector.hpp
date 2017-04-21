//
// Created by Richard Hodges on 20/04/2017.
//

#include <quippy/config.hpp>
#include <quippy/connector_service.hpp>


namespace quippy {

    struct connector {
        using service_type = connector_service;
        using implementation_type = service_type::implementation_type;

        connector(asio::io_service &ios)
            : service_ptr_(std::addressof(asio::use_service<service_type>(ios))), impl_(get_service().create()) {}

        void connect(asio::ip::tcp::resolver::iterator iter) {
            get_service().connect(get_implementation(), iter);
        }

        service_type &get_service() const { return *service_ptr_; }

        implementation_type &get_implementation() { return impl_; }

        implementation_type const &get_implementation() const { return impl_; }

    private:
        service_type *service_ptr_;
        implementation_type impl_;
    };
}