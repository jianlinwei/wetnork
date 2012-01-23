#ifndef SIGNAL_H
#define SIGNAL_H

#include <boost/signals2.hpp>

template<typename Signature>
using Signal = typename boost::signals2::signal_type<
	Signature,
	boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>
	>::type;

using SignalConnection = boost::signals2::connection;

#endif
