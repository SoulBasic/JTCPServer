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
			std::cout << "与服务器断开连接" << std::endl;
			break;
		}

		switch (header.CMD)
		{
		case CMD_PRIVATEMESSAGE:
		{
			PrivateMessagePack pack = tc->receive<PrivateMessagePack>();
			std::cout << "收到用户" << pack.targetName << "发来的私信：" << pack.message << std::endl;
			break;
		}
		case CMD_MESSAGE:
		{
			MessagePack pack = tc->receive<MessagePack>();
			std::cout << "接收到服务器的消息:" << pack.message << std::endl;
			break;
		}
		case CMD_BROADCAST:
		{
			BroadcastPack pack = tc->receive<BroadcastPack>();
			std::cout << "接收到广播消息:" << pack.message << std::endl;
			break;
		}
		case CMD_NAME:
		{
			NamePack pack = tc->receive<NamePack>();
			std::cout << "您被服务器更名为:" << pack.name << std::endl;
			
		}
		default:
		{
			std::cout << "无法解析的消息:CMD=" << header.CMD <<" length="<<header.LENGTH<< std::endl;
			break;
		}
		}
			
	}
}


int main()
{
	TCPClient tc;
	tc.initSocket();
	if (-1 == tc.connectServer("127.0.0.1", 2324))
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
			std::cout << "请输入私信对象：";
			std::cin >> cmd;
			strcpy(pack.targetName, cmd.c_str());
			std::cout << "请输入私信内容：";
			std::cin >> cmd;
			strcpy(pack.message, cmd.c_str());
			tc.sendMessage(pack);
		}
		else if (cmd == "bc")
		{
			BroadcastPack pack;
			std::cout << "请输入广播内容：";
			std::cin >> cmd;
			strcpy_s(pack.message, cmd.c_str());
			tc.sendMessage(pack);
		}
		else if (cmd == "name")
		{
			NamePack pack;
			std::cout << "请输入您的昵称：";
			std::cin >> cmd;
			strcpy_s(pack.name, cmd.c_str());
			tc.sendMessage(pack);
		}
		

	}
	std::cout << "程序结束" << std::endl;
	tc.terminal();
	::system("pause");
}