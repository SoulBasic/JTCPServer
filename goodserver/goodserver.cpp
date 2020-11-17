
#include <iostream>
#include "selectTCPServer.hpp"
bool running = false;
void cmdThread(TCPServer* s)
{
	std::string cmd = "";
	while (running)
	{
		std::cin >> cmd;
		if ("quit" == cmd)
		{
			running = false;
			break;
		}
		else
		{
			std::cout << "未定义的命令" << std::endl;
		}

	}
}
int main()
{
	TCPServer server;
	server.initSocket();
	server.bindServer("127.0.0.1", 2324, 10);
	running = true;
	std::thread tcmd(cmdThread, &server);
	tcmd.detach();
	while (server.active() && running)
	{
		server.OnRun();
	}
}
