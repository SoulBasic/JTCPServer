#include <iostream>
#include "TCPClient.hpp"
#include <thread>
#include <string>

bool isRun = false;

void getMessage(TCPClient* tc)
{
	while (true)
	{
		Header header = tc->receive<Header>();
		if (header.CMD <= 0)
		{
			std::cout << "Disconnect from server" << std::endl;
			break;
		}

		switch (header.CMD)
		{
		case CMD_PRIVATEMESSAGE:
		{
			PrivateMessagePack pack = tc->receive<PrivateMessagePack>();
			std::cout << "Received " << pack.targetName << " Private message：" << pack.message << std::endl;
			break;
		}
		case CMD_MESSAGE:
		{
			MessagePack pack = tc->receive<MessagePack>();
			std::cout << "Message received from server:" << pack.message << std::endl;
			break;
		}
		case CMD_BROADCAST:
		{
			BroadcastPack pack = tc->receive<BroadcastPack>();
			std::cout << "Received broadcast message:" << pack.message << std::endl;
			break;
		}
		case CMD_NAME:
		{
			NamePack pack = tc->receive<NamePack>();
			std::cout << "You have been renamed by the server to:" << pack.name << std::endl;
			
		}
		default:
		{
			std::cout << "Unresolved message:CMD=" << header.CMD <<" length="<<header.LENGTH<< std::endl;
			break;
		}
		}
			
	}
}


int main()
{
	TCPClient tc;
	tc.initSocket();
	if (-1 == tc.connectServer("192.168.199.132", 2324))
	{
		return -1;
	}
	std::thread t1(getMessage, &tc);
	while (true)
	{
		std::string cmd;
		std::cin >> cmd;

		if (cmd == "cpm")
		{
			PrivateMessagePack pack;
			std::cout << "Please enter private message targetName：";
			std::cin >> cmd;
			strcpy(pack.targetName, cmd.c_str());
			std::cout << "Please enter the private message：";
			std::cin >> cmd;
			strcpy(pack.message, cmd.c_str());
			tc.sendMessage(pack);
		}
		else if (cmd == "bc")
		{
			BroadcastPack pack;
			std::cout << "Please input broadcast content：";
			std::cin >> cmd;
			strcpy(pack.message, cmd.c_str());
			tc.sendMessage(pack);
		}
		else if (cmd == "name")
		{
			NamePack pack;
			std::cout << "Enter your nickname ：";
			std::cin >> cmd;
			strcpy(pack.name, cmd.c_str());
			tc.sendMessage(pack);
		}
		else
		{
			MessagePack pack;
			strcpy(pack.message, cmd.c_str());
			tc.sendMessage(pack);
		}
		

	}
	std::cout << "End of procedure" << std::endl;
	tc.terminal();
	::system("pause");
}