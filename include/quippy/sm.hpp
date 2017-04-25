//
// Created by Richard Hodges on 22/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <tuple>
#include <utility>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>

#include <boost/signals2.hpp>


namespace quippy {
    namespace notstd {
        template<class T, class Tuple>
        struct tuple_index;

        template<class T, class... Types>
        struct tuple_index<T, std::tuple<T, Types...>> {
            static const std::size_t value = 0;
        };

        template<class T, class U, class... Types>
        struct tuple_index<T, std::tuple<U, Types...>> {
            static const std::size_t value = 1 + tuple_index<T, std::tuple<Types...>>::value;
        };

        template<class T>
        struct category {
            static std::string name() { return typeid(T).name(); }
        };

        template<class T>
        struct category<const T> {
            static std::string name() { return category<T>::name() + " const"; }
        };

        template<class T>
        struct category<T &> {
            static std::string name() { return category<T>::name() + "&"; }
        };

        template<class T>
        struct category<T &&> {
            static std::string name() { return category<T>::name() + "&&"; }
        };

        template<class T>
        struct category<T *> {
            static std::string name() { return category<T>::name() + "*"; }
        };

        namespace detail {
            template<typename T, template<typename...> class Tmpl>  // #1 see note
            struct is_derived_from_template {
                static std::false_type test(...);

                template<typename...U>
                static std::true_type test(Tmpl<U...> const &);

                using result = decltype(test(std::declval<T>()));
                using value_type = typename result::value_type;
                static constexpr value_type value = result::value;
            };
        }
        template<typename T, template<typename...> class Tmpl>  // #1 see note
        using is_derived_from_template = typename detail::is_derived_from_template<T, Tmpl>::result;

        template<typename T, template<typename...> class Tmpl>  // #1 see note
        static constexpr auto is_derived_from_template_v = is_derived_from_template<T, Tmpl>::value;


        namespace detail {
            template<class F, class Tuple, std::size_t... I>
            constexpr void for_all_impl(F &&f, Tuple &&t, std::index_sequence<I...>) {
                using expand = int[];
                void(expand{0,
                            (f(std::get<I>(std::forward<Tuple>(t))), 0)...
                });
            }
        }  // namespace detail

        template<class F,
            class Tuple,
            std::enable_if_t<is_derived_from_template_v<Tuple, std::tuple> > * = nullptr
        >
        constexpr void for_all(F &&f, Tuple &&t) {
            return detail::for_all_impl(std::forward<F>(f), std::forward<Tuple>(t),
                                        std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
        }


    }

    namespace detail {

        struct dispatch_helper {
            template<std::size_t N> using index = std::integral_constant<std::size_t, N>;

            template<class Tuple, class F>
            void operator()(Tuple &&tuple, std::size_t i, F &&f) const {
                return dispatch(std::forward<Tuple>(tuple), index<0>(), i, std::forward<F>(f));
            }

            template<
                class Tuple,
                std::size_t I,
                class F,
                std::enable_if_t<(std::tuple_size<std::decay_t<Tuple>>::value > I)> * = nullptr
            >
            void dispatch(Tuple &&tuple, index<I>, std::size_t i, F &&f) const {
                if (i == I)
                    return f(std::get<I>(std::forward<Tuple>(tuple)));
                else
                    return dispatch(std::forward<Tuple>(tuple), index<I + 1>(), i, std::forward<F>(f));
            };

            template<
                class Tuple,
                std::size_t N,
                class F,
                std::enable_if_t<std::tuple_size<std::decay_t<Tuple>>::value == N> * = nullptr
            >
            void dispatch(Tuple &&tuple, index<N>, std::size_t i, F &&f) const {
                assert(!"call out of bounds");
            };

        };

    }

    template<class Tuple, class Function>
    decltype(auto) tuple_dispatch(Tuple &&tuple, std::size_t index, Function &&f) {
        auto helper = detail::dispatch_helper();
        return helper(std::forward<Tuple>(tuple), index, std::forward<Function>(f));
    }


    namespace msm = boost::msm;
    namespace msmf = boost::msm::front;
    namespace mpl = boost::mpl;

    namespace sig = boost::signals2;
    using subscription = sig::scoped_connection;


    // default back end for this project
    template<class FrontEnd>
    struct to_back_end {
        using type = msm::back::state_machine<FrontEnd>;
    };

    template<class FrontEnd>
    using to_back_end_t = typename to_back_end<FrontEnd>::type;

    template<class Outer>
    struct logging_state : msmf::state<>
    {

        using base_state = logging_state<Outer>;

        template<class Event, class FSM>
        void on_entry(Event const &event, FSM &fsm) {
            QUIPPY_LOG(debug) << "FSM: " << typeid(FSM).name() << " ENTER STATE: " << typeid(Outer).name()
                              << " WITH EVENT: " << typeid(Event).name();
        }

        template<class Event, class FSM>
        void on_exit(Event const &event, FSM &fsm) {
            QUIPPY_LOG(debug) << "FSM: " << typeid(FSM).name() << " EXIT STATE : " << typeid(Outer).name()
                              << " WITH EVENT: " << typeid(Event).name();
        }
    };

    template<class Outer>
    struct non_logging_state : msmf::state<>
    {
        using base_state = non_logging_state<Outer>;

    };

    template<class State> using base_state = logging_state<State>;

}