#include <iostream>
#include "TCPClient.hpp"
#include <thread>
#include <string>

bool isRun = false;



void getMessage(TCPClient* tc)
{
	while (true)
	{
		Header header;
		if (!tc->receive<Header>(header))
		{
			std::cout << "Disconnected form server" << std::endl;
			break;
		}
		switch (header.CMD)
		{
		case CMD_PRIVATEMESSAGE:
		{
			PrivateMessagePack pack;
			tc->receive<PrivateMessagePack>(pack);
			std::cout << "Received " << pack.targetName << " Private message£º" << pack.message << std::endl;
			break;
		}
		case CMD_MESSAGE:
		{
			MessagePack pack;
			tc->receive<MessagePack>(pack);
			std::cout << "Message received from server:" << pack.message << std::endl;
			break;
		}
		case CMD_BROADCAST:
		{
			BroadcastPack pack;
			tc->receive<BroadcastPack>(pack);
			std::cout << "Received broadcast message:" << pack.message << std::endl;
			break;
		}
		case CMD_NAME:
		{
			NamePack pack;
			tc->receive<NamePack>(pack);
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
	TCPClient tc("127.0.0.1", 2324);
	tc.initSocket();
	if (-1 == tc.connectServer())
	{
		return -1;
	}
	std::thread t1(getMessage, &tc);
	//int count = 0;
	//while (true)
	//{
	//	std::string cmd = std::to_string(count++);
	//	MessagePack pack;
	//	strcpy(pack.message, cmd.c_str());
	//	tc.sendMessage(pack);
	//	Sleep(10);
	//	if (count >= 10000)break;
	//}
	
	while (true)
	{
		std::string cmd;
		std::cin >> cmd;

		if (cmd == "cpm")
		{
			PrivateMessagePack pack;
			std::cout << "Please enter private message targetName£º";
			std::cin >> cmd;
			strcpy(pack.targetName, cmd.c_str());
			std::cout << "Please enter the private message£º";
			std::cin >> cmd;
			strcpy(pack.message, cmd.c_str());
			tc.sendMessage(pack);
		}
		else if (cmd == "bc")
		{
			BroadcastPack pack;
			std::cout << "Please input broadcast content£º";
			std::cin >> cmd;
			strcpy(pack.message, cmd.c_str());
			tc.sendMessage(pack);
		}
		else if (cmd == "name")
		{
			NamePack pack;
			std::cout << "Enter your nickname £º";
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