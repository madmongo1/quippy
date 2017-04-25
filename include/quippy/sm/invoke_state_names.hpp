#pragma once

#include <quippy/config.hpp>
#include <quippy/sm/config.hpp>
#include <boost/msm/back/tools.hpp>
namespace quippy {

    struct invoke_state_names
    {
        template< class BackEnd >
        std::string operator()(BackEnd &backend) const
        {
            using back_end_type = BackEnd;
            using Stt = typename back_end_type::stt;
            using all_states = typename msm::back::generate_state_set<Stt>::type;
            char const *state_names[mpl::size<all_states>::value];
            // fill the names of the states defined in the state machine
            mpl::for_each<all_states, boost::msm::wrap<mpl::placeholders::_1> >
            (msm::back::fill_state_names<Stt>(state_names));

            std::string result;

            for (unsigned int i = 0; i < back_end_type::nr_regions::value; ++i) {
                if (i)
                    result += ", ";
                result += state_names[backend.current_state()[i]];
            }

            return result;
        }
    };

}
