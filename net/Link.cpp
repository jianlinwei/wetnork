#include "network.hpp"

Link::~Link()
{
}

void Link::setState(State state)
{
	this->_state = state;
	stateChanged(*this, state);
}

SignalConnection Link::connectStateChanged(OnStateChanged::slot_function_type cb)
{
	if (!stateChanged.empty()) {
		throw InvalidOperation("StateChanged already connected");
	}

	return stateChanged.connect(cb);
}

Link::State Link::state() const
{
	return _state;
}
