#include "network.hpp"

Link::~Link()
{
}

void Link::setState(State state)
{
	State oldState = _state;
	this->_state = state;
	stateChanged(*this, oldState);
}

bs2::connection Link::connectStateChanged(OnStateChanged::slot_function_type cb)
{
	return stateChanged.connect(cb);
}

bs2::connection Link::connectReceive(OnReceive::slot_function_type cb)
{
	return receive.connect(cb);
}

Link::State Link::state() const
{
	return _state;
}
