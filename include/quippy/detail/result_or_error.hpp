//
// Created by Richard Hodges on 21/04/2017.
//

#pragma once
#include <quippy/config.hpp>
#include <boost/variant.hpp>
#include <type_traits>

namespace quippy { namespace detail {

    struct indicate_void {};
    struct indicate_nothing {};

    template<class T> struct result_or_error
    {
        using value_type = T;
        using store_type = boost::variant<indicate_nothing, value_type, asio::error_code>;

        result_or_error()
            : store_(indicate_nothing())
        {}

        template<class Arg>
        void set_value(Arg&& value)
        {
            static_assert(std::is_convertible<Arg, value_type>::value, "");
            auto lock = std::unique_lock<std::mutex>(mutex_);
            if (store_.type() != typeid(indicate_nothing))
            {
                throw std::logic_error("already has value");
            }
            store_ = std::forward<Arg>(value);
            lock.unlock();
            cv_.notify_all();
        }

        void set_error(asio::error_code const& ec)
        {
            auto lock = std::unique_lock<std::mutex>(mutex_);
            if (store_.type() != typeid(indicate_nothing))
            {
                throw std::logic_error("already has value");
            }
            store_ = ec;
            lock.unlock();
            cv_.notify_all();
        }

        void wait() {
            auto lock = std::unique_lock<std::mutex>(mutex_);
            cv_.wait(lock, [this]{
                return this->store_.type() != typeid(indicate_nothing);
            });
        }

        /// @pre wait() has been called
        bool has_error() const {
            wait();
            return store_.type() == typeid(asio::error_code);
        }

        /// @pre wait() has been called
        asio::error_code error() const {
            wait();
            struct visitor : boost::static_visitor<asio::error_code>
            {
                asio::error_code operator()(indicate_nothing) const {
                    throw std::logic_error("error");
                }

                asio::error_code operator()(value_type const&) const {
                    return asio::error_code();
                }

                asio::error_code operator()(asio::error_code const& ec) const {
                    return ec;
                }
            };
            return boost::apply_visitor(visitor(), store_);
        }

        /// @pre wait() has been called
        value_type get() {
            wait();
            struct visitor : boost::static_visitor<value_type>
            {
                value_type operator()(indicate_nothing) const {
                    throw std::logic_error("get");
                }

                value_type operator()(value_type& value) const {
                    return std::move(value);
                }

                value_type operator()(asio::error_code const& ec) const {
                    throw asio::system_error(ec);
                }
            };
            return boost::apply_visitor(visitor(), store_);
        }



        store_type store_;
        std::condition_variable cv_;
        std::mutex mutex_;
    };

    template<> struct result_or_error<void>
    {
        using value_type = void;

        void set_error(const asio::error_code& ec) {
            impl_.set_error(ec);
        }

        void set_value() {
            impl_.set_value(indicate_void());
        }

        void get() {
            void(impl_.get());
        }

        void wait() {
            impl_.wait();
        }

        void set_value_or_error(asio::error_code const& ec) {
            if (ec) {
                set_error(ec);
            }
            else {
                set_value();
            }
        }


        result_or_error<indicate_void> impl_;
    };

}}