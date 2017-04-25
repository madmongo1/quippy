//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/signals2.hpp>

namespace quippy
{
    namespace msm = boost::msm;
    namespace msmf = boost::msm::front;
    namespace mpl = boost::mpl;

    namespace sig = boost::signals2;
    using subscription = sig::scoped_connection;

}