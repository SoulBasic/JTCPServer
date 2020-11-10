
#include <iostream>
#include "selectTCPServer.hpp"


int main()
{
	TCPServer server;
	server.initSocket();
	server.bindServer("10.4.246.94", 2324, 10);

	while (server.active())
	{
		server.OnRun();
	}
}
