#pragma once
#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_

#include "Client.hpp"
class INetEvent 
{
public:
	virtual void OnLeave(std::shared_ptr<CLIENT> c) = 0;
};
#endif