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
			std::cout << "收到他人发来的私信：" << pack.message << std::endl;
			break;
		}
		case CMD_MESSAGE:
		{
			MessagePack pack = tc->receive<MessagePack>();
			std::cout << "接收到服务器的消息:CMD=" << header.CMD << " LENGTH=" << header.LENGTH << " DATA=" << pack.message << std::endl;
			break;
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
			pack.targetUID = atoi(cmd.c_str());
			std::cout << "请输入私信内容：";
			std::cin >> cmd;
			strcpy(pack.message, cmd.c_str());
			tc.sendMessage(pack);
		}
		else
		{
			MessagePack pack;
			strcpy(pack.message, cmd.c_str());
			tc.sendMessage(pack);
		}
		

	}
	tc.terminal();
	::system("pause");
}