#include "network.hpp"

Link::~Link()
{
}

void Link::setState(LinkState state)
{
	this->_state = state;
	onStateChanged(*this, state);
}

LinkState Link::state() const
{
	return _state;
}
