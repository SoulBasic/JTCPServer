#include <iostream>
#include "TCPServer.hpp"
#include <thread>
#include <string>
#include <algorithm>
#include <Jutil.h>
#include <unordered_map>


std::vector<CLIENT> clients;
std::unordered_map<std::string, int>users;
void service(TCPServer* server, CLIENT client)
{
	SOCKET csock = std::get<0>(client);
	sockaddr_in csin = std::get<1>(client);
	{//欢迎
		MessagePack pack;
		std::string userName = "user";
		for (auto& i : users)
		{
			if (i.second == csock)
			{
				userName = i.first;
				break;
			}
		}
		strcpy_s(pack.message, userName.c_str());
		server->sendMessage(csock, pack);
	}
	while (true)
	{
		Header header = server->receive<Header>(csock);
		if (header.CMD <= 0)
		{
			std::cout << "客户" << csock << "与服务器断开连接" << std::endl;
			break;
		}
		switch (header.CMD)
		{
		case CMD_PRIVATEMESSAGE:
		{
			PrivateMessagePack pack = server->receive<PrivateMessagePack>(csock);
			csock = pack.targetUID;
			pack.CMD = header.CMD;
			pack.LENGTH = header.LENGTH;
			std::cout << "转发私信" << std::endl;
			server->sendMessage(csock, pack);
			break;
		}
		case CMD_MESSAGE:
		{
			MessagePack pack = server->receive<MessagePack>(csock);
			std::cout << "接收到客户端的消息:CMD=" << header.CMD << " LENGTH=" << header.LENGTH << " DATA=" << pack.message << std::endl;
			pack.CMD = CMD_MESSAGE;
			pack.LENGTH = 33;
			strcpy_s(pack.message, "好的服务器已经收到了您的消息了！");
			server->sendMessage(csock, pack);
			break;
		}
		default:
		{
			std::cout << "无法解析的消息:CMD=" << header.CMD << std::endl;
			break;
		}
		}
	}

}


int main()
{
	std::vector<std::thread> threads;
	TCPServer server;
	if (!server.initSocket())return -1;
	if (!server.bindServer("127.0.0.1", 2324))return -1;
	while (true)
	{
		CLIENT client = server.acceptClient();
		users.insert(std::make_pair("user" + std::to_string(std::get<0>(client)), std::get<0>(client)));
		threads.push_back(std::thread(service, &server, client));
	}
	server.terminal();
	return 0;
}