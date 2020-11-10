
#include <iostream>
#include "selectTCPServer.hpp"


int main()
{
	TCPServer server;
	server.initSocket();
	server.bindServer("127.0.0.1", 2324, 10);

	while (server.active())
	{
		server.OnRun();
	}
}
