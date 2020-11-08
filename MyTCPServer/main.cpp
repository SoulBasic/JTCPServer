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
		Jutil::getKey<std::string, int>(users, csock, userName);
		userName = "欢迎加入,您的昵称是 " + userName;
		strcpy(pack.message, userName.c_str());
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
			pack.CMD = header.CMD;
			pack.LENGTH = header.LENGTH;
			std::cout << "转发私信" << std::endl;
			auto it = users.find(std::string(pack.targetName));

			std::string sourceName = "user";
			Jutil::getKey<std::string, int>(users, csock, sourceName);
			strcpy(pack.targetName, sourceName.c_str());
			if (it != users.end())
			{
				server->sendMessage((*it).second, pack);
			}

			break;
		}
		case CMD_MESSAGE:
		{
			MessagePack pack = server->receive<MessagePack>(csock);
			pack.CMD = header.CMD;
			pack.LENGTH = header.LENGTH;
			std::cout << "接收到客户端的消息:CMD=" << header.CMD << " LENGTH=" << header.LENGTH << " DATA=" << pack.message << std::endl;
			strcpy(pack.message, "好的服务器已经收到了您的消息了！");
			server->sendMessage(csock, pack);
			break;
		}
		case CMD_BROADCAST:
		{
			BroadcastPack pack = server->receive<BroadcastPack>(csock);
			std::cout << "广播消息" << std::endl;
			pack.CMD = header.CMD;
			pack.LENGTH = header.LENGTH;
			for (int i = 0; i < clients.size(); i++)
			{
				server->sendMessage(std::get<0>(clients[i]), pack);
			}
			break;
		}
		case CMD_NAME:
		{
			NamePack pack = server->receive<NamePack>(csock);
			std::string userName = "";
			bool find = false;
			for (auto& i : users)
			{
				if (i.second == csock)
				{
					userName = i.first;
					users.erase(userName);
					userName = pack.name;
					users.insert(std::make_pair(userName, csock));
					find = true;
					break;
				}
			}
			if (find)
			{
				MessagePack pack;
				userName = "更名成功，现在的昵称为" + userName;
				strcpy(pack.message, userName.c_str());
				server->sendMessage(csock, pack);
			}
			else
			{
				MessagePack pack;
				userName = "更名失败";
				strcpy(pack.message, userName.c_str());
				server->sendMessage(csock, pack);
			}
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
		clients.push_back(client);
		threads.push_back(std::thread(service, &server, client));
	}
	server.terminal();
	return 0;
}