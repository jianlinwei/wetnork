#include "network.hpp"

Link::~Link()
{
}

void Link::setState(State state)
{
	this->_state = state;
	stateChanged(*this, state);
}

bs2::connection Link::connectStateChanged(OnStateChanged::slot_function_type cb)
{
	return stateChanged.connect(cb);
}

Link::State Link::state() const
{
	return _state;
}
