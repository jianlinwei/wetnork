#ifndef SIGNAL_H
#define SIGNAL_H

#include <more/signals.hpp>

namespace ms = more::signals;

template<typename Signature>
using Signal = more::signals::signal<Signature>;

#endif
