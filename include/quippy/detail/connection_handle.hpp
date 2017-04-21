#include <quippy/config.hpp>
#include <type_traits>
#include <memory>

namespace quippy { namespace detail {
/*
    struct connection_handle {

        using native_type = std::remove_pointer_t<amqp_connection_state_t>;

        connection_handle()
            : ptr_{ amqp_new_connection() }
        {}


    private:
        struct deleter
        {
            void operator()(amqp_connection_state_t p) const noexcept {
                amqp_destroy_connection(p);
            }
        };

        using ptr_type = std::unique_ptr<native_type, deleter>;

        ptr_type ptr_;
    };
    */
}}

