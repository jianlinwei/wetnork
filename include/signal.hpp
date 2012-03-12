#ifndef SIGNAL_H
#define SIGNAL_H

#include <boost/signals2.hpp>

namespace bs2 = boost::signals2;

template<typename Signature>
using Signal = typename boost::signals2::signal_type<
	Signature,
	boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>
	>::type;

#endif
