#include "network.hpp"

Link::~Link()
{
}

void Link::setState(LinkState state)
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

LinkState Link::state() const
{
	return _state;
}
