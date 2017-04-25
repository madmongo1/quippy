//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

namespace quippy { namespace detail {

    template<class Service>
    struct io_object
    {
        using service_type = Service;

        using implementation_type = typename service_type ::implementation_type;

        template<class...Args>
        io_object(asio::io_service& owner, Args&&...args)
        : service_(std::addressof(asio::use_service<service_type>(owner)))
        , impl_(get_service().create(std::forward<Args>(args)...))
        {}

        io_object(io_object const&) = delete;
        io_object(io_object&&) = default;

        ~io_object()
        {
            get_service().destroy(get_implemenentation());
        }

        implementation_type & get_implemenentation() { return impl_; }

        implementation_type const & get_implemenentation() const { return impl_; }

        service_type& get_service() const { return *service_; }

        asio::io_service& get_io_service() { return get_service().get_io_service(); }
    private:
        service_type* service_;
        implementation_type impl_;

    };
}}