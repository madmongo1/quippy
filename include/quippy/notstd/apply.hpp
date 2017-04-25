//
// Created by Richard Hodges on 23/04/2017.
//

#pragma once
#include <quippy/notstd/config.hpp>
#include <quippy/notstd/invoke.hpp>

namespace quippy {

    namespace notstd {

        namespace detail {
            template <class F, class Tuple, std::size_t... I>
            constexpr decltype(auto) apply_impl(F &&f, Tuple &&t, std::index_sequence<I...>)
            {
                return invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
            }
        }  // namespace detail

        template <class F, class Tuple>
        constexpr decltype(auto) apply(F &&f, Tuple &&t)
        {
            return detail::apply_impl(
                std::forward<F>(f), std::forward<Tuple>(t),
                std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
        }
    }
}
