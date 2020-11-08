#include <iostream>
#include "TCPServer.hpp"
#include <thread>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <algorithm>

//·Ö¸î×Ö·û´®
std::vector<std::string> split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;//À©Õ¹×Ö·û´®ÒÔ·½±ã²Ù×÷
	int size = str.size();
	for (int i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

//unordered_map valueÕÒkey
template<typename keyT, typename valueT>
bool getKey(std::unordered_map<keyT, valueT>& map, valueT val, keyT& key)
{
	for (auto& i : map)
	{
		if (i.second == val)
		{
			key = i.first;
			return true;
		}
	}
	return false;
}

std::vector<CLIENT> clients;
std::unordered_map<std::string, int>users;
void service(TCPServer* server, CLIENT client)
{
	SOCKET csock = std::get<0>(client);
	sockaddr_in csin = std::get<1>(client);
	{//»¶Ó­
		MessagePack pack;
		std::string userName = "user";
		getKey<std::string, int>(users, csock, userName);
		userName = "Welcome to join. Your nickname is " + userName;
		strcpy(pack.message, userName.c_str());
		server->sendMessage(csock, pack);
	}
	while (true)
	{
		Header header;
		if (!server->receive<Header>(csock, header))
		{
			std::cout << "client " << csock << " Disconnect from server" << std::endl;
			for (auto it = clients.begin(); it < clients.end(); it++)
			{
				if (std::get<0>(*it) == csock)
				{
					clients.erase(it);
					break;
				}
			}
			std::string name;
			getKey<std::string, int>(users, csock, name);
			users.erase(name);
			break;
		}
		switch (header.CMD)
		{
		case CMD_PRIVATEMESSAGE:
		{
			PrivateMessagePack pack;
			server->receive<PrivateMessagePack>(csock, pack);
			pack.CMD = header.CMD;
			pack.LENGTH = header.LENGTH;
			std::cout << "Forward private message " << std::endl;
			auto it = users.find(std::string(pack.targetName));

			std::string sourceName = "user";
			getKey<std::string, int>(users, csock, sourceName);
			strcpy(pack.targetName, sourceName.c_str());
			if (it != users.end())
			{
				server->sendMessage((*it).second, pack);
			}

			break;
		}
		case CMD_MESSAGE:
		{
			MessagePack pack;
			server->receive<MessagePack>(csock, pack);
			pack.CMD = header.CMD;
			pack.LENGTH = header.LENGTH;
			std::cout << "Message received from client :CMD=" << header.CMD << " LENGTH=" << header.LENGTH << " DATA=" << pack.message << std::endl;
			strcpy(pack.message, "OK, the server has received your message!");
			server->sendMessage(csock, pack);
			break;
		}
		case CMD_BROADCAST:
		{
			BroadcastPack pack;
			server->receive<BroadcastPack>(csock, pack);
			std::cout << "Broadcast news" << std::endl;
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
			NamePack pack;
			server->receive<NamePack>(csock, pack);
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
				userName = "The name has been changed successfully, and the nickname is now " + userName;
				strcpy(pack.message, userName.c_str());
				server->sendMessage(csock, pack);
			}
			else
			{
				MessagePack pack;
				userName = "Failed to rename";
				strcpy(pack.message, userName.c_str());
				server->sendMessage(csock, pack);
			}
			break;
		}
		default:
		{
			std::cout << "Unresolved message:CMD=" << header.CMD << std::endl;
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