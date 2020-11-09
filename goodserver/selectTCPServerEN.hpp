#ifndef _SELECTTCPServer_HPP_
#define _SELECTTCPServer_HPP_

#include <iostream>
#include <tuple>
#include <vector>
#include <algorithm>

#include <string>
#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#include "../Pack.hpp"
#else//Linux
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "Pack.hpp"	
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif 

#define CMD_ERROR 0
#define CMD_SUCCESS 1

#define CLIENT_DISCONNECT -1

#define RECV_BUF_SIZE 40960
#define MSG_BUF_SIZE 409600

class CLIENT
{
public:
	CLIENT(SOCKET csock, sockaddr_in csin, int userid, std::string username)
		:sock(csock), sin(csin), lastBufPos(0), userID(userid), userName(username) {}
	inline SOCKET getSock() { return sock; }
	inline sockaddr_in getSin() { return sin; }
	inline char* getmsgBuf() { return msgBuf; }
	inline int getLastBufPos() { return lastBufPos; }
	inline void setLastBufPos(int val) { lastBufPos = val; }
	inline int getUserID() { return userID; }
	inline std::string getUserName() { return userName; }
	inline void setUserName(std::string username) { userName = username; }
private:
	SOCKET sock;
	sockaddr_in sin;
	char msgBuf[MSG_BUF_SIZE] = {};
	int lastBufPos;
	int userID;
	std::string userName;
};


class TCPServer
{
private:
	SOCKET ssock;
	sockaddr_in ssin;
	fd_set fdRead;
	fd_set fdWrite;
	fd_set fdExp;

	char recvBuf[RECV_BUF_SIZE] = {};
public:
	std::vector<CLIENT*> clients;
	inline SOCKET getSocket() { return ssock; }
	inline sockaddr_in getSockaddr_in() { return ssin; }
	TCPServer(const TCPServer& other) = delete;
	const TCPServer& operator=(const TCPServer& other) = delete;
public:

	explicit inline TCPServer()
	{
		ssock = INVALID_SOCKET;
		ssin = {};

	}

	~TCPServer()
	{
		terminal();
	}


	inline bool active() { return ssock != INVALID_SOCKET; }


	int initSocket()
	{
#ifdef _WIN32
		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (SOCKET_ERROR == WSAStartup(version, &data))
		{
			std::cout << "Failed to initialize Winsock environment" << std::endl;
		}
		else
		{
			std::cout << "Winsock environment initialized successfully!" << std::endl;
		}
#endif 
		ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == ssock)
		{
			std::cout << "Failed to initialize the server" << std::endl;
			return CMD_ERROR;
		}
		else
		{
			std::cout << "Server initialization successful!" << std::endl;
		}
		return CMD_SUCCESS;
	}

	//°ó¶¨²¢¼àÌý¶Ë¿Ú
	int bindServer(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == ssock)
		{
			std::cout << "Socket is not initialized or invalid" << std::endl;
			return CMD_ERROR;
		}
		ssin.sin_family = AF_INET;
		ssin.sin_port = htons(port);
#ifdef _WIN32
		ssin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		ssin.sin_addr.s_addr = inet_addr(ip);
#endif 
		int res = bind(ssock, (sockaddr*)&ssin, sizeof(ssin));
		if (SOCKET_ERROR == res)
		{
			std::cout << "Failed to bind port" << std::endl;
			return CMD_ERROR;
		}
		else
		{
			std::cout << "Port bound successfully!" << std::endl;
		}

		if (SOCKET_ERROR == listen(ssock, 5))
		{
			std::cout << "Listening port failed" << std::endl;
			return -1;
		}
		else
		{
			std::cout << "Listening port succeeded!" << std::endl;
		}

		return CMD_SUCCESS;
	}


	bool OnRun()
	{
		if (!active())return false;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		FD_SET(ssock, &fdRead);
		FD_SET(ssock, &fdWrite);
		FD_SET(ssock, &fdExp);
		SOCKET maxSocket = ssock;
		for (auto c : clients)
		{
			FD_SET(c->getSock(), &fdRead);
			maxSocket = std::max(maxSocket, c->getSock());
		}
		timeval t = { 1,0 };
		int res = select(maxSocket + 1, &fdRead, &fdWrite, &fdExp, &t);
		//std::cout << "select result = " << res << " num = " << num++ << std::endl;
		if (res < 0)
		{
			std::cout << "Select model unknown error, task ended" << std::endl;
			terminal();
			return false;
		}
		if (FD_ISSET(ssock, &fdRead))
		{
			acceptClient();
			FD_CLR(ssock, &fdRead);
			return true;
		}

		for (auto i : clients)
		{
			if (FD_ISSET(i->getSock(), &fdRead))
			{
				recvPack(i);
			}

		}
		return true;
	}


	template<typename PackType>
	int sendMessage(SOCKET csock, PackType* msg)
	{
		if (INVALID_SOCKET == ssock)
		{
			std::cout << "The server socket is not initialized or invalid" << std::endl;
			return CMD_ERROR;
		}
		int res = send(csock, (const char*)msg, sizeof(PackType), 0);
		if (SOCKET_ERROR == res)
		{
			std::cout << "Sending packet failed" << std::endl;
			return CMD_ERROR;
		}
		else
		{

			//std::cout << "Send packets to(" << csock << " ) " << msg->CMD << " " << msg->LENGTH << std::endl;
		}
		return CMD_SUCCESS;
	}


	void terminal()
	{
		if (INVALID_SOCKET == ssock)return;
#ifdef _WIN32
		for (CLIENT* c : clients)
		{
			closesocket(c->getSock());
			delete c;
		}
		closesocket(ssock);
		WSACleanup();
#else //Linux
		for (CLIENT* c : clients)
		{
			close(c->getSock());
			delete c;
		}
		close(ssock);
#endif 
		ssock = INVALID_SOCKET;
		clients.clear();
	}

	virtual void handleMessage(CLIENT* c, Pack* pk)
	{
		switch (pk->CMD)
		{
		case CMD_PRIVATEMESSAGE:
		{
			PrivateMessagePack* pack = static_cast<PrivateMessagePack*>(pk);
			std::cout << "Forward private message " << std::endl;
			std::string sourceName = "user";
			SOCKET target = 0;
			auto it = clients.begin();
			for (it; it < clients.end(); it++)
			{
				if ((*it)->getUserName() == pack->targetName)
				{
					target = (*it)->getSock();
					break;
				}
			}

			strcpy(pack->targetName, c->getUserName().c_str());
			if (it != clients.end())
			{
				sendMessage(target, pack);
			}
			else
			{
				MessagePack pack1;
				strcpy(pack1.message, "Failed to send private message. The target user does not exist or is offline");
				sendMessage(c->getSock(), &pack1);
			}
			break;
		}
		case CMD_MESSAGE:
		{
			MessagePack* pack = static_cast<MessagePack*>(pk);
			std::cout << "From client(" << c->getSock() << ")Message received :CMD=" << pack->CMD << " LENGTH=" << pack->LENGTH << " DATA=" << pack->message << std::endl;
			strcpy(pack->message, "Message received by server successfully!");
			sendMessage(c->getSock(), pack);
			break;
		}
		case CMD_BROADCAST:
		{
			BroadcastPack* pack = static_cast<BroadcastPack*>(pk);
			std::cout << "Broadcast news" << std::endl;
			for (auto c1 : clients)
			{
				sendMessage(c1->getSock(), pack);
			}
			break;
		}
		case CMD_NAME:
		{
			NamePack* pack = static_cast<NamePack*>(pk);
			std::string userName = "";
			std::string oldName = "";
			auto it = clients.begin();
			for (it; it < clients.end(); it++)
			{
				if (c->getSock() == (*it)->getSock())
				{
					oldName = (*it)->getUserName();
					(*it)->setUserName(pack->name);
					userName = pack->name;
					break;
				}
			}
			if (it != clients.end())
			{

				std::cout << "user " << oldName << "(" << c->getSock() << ")Renamed as" << userName << std::endl;
				userName = "The name has been successfully changed, now nickname is " + userName;
				MessagePack pack1(userName.c_str());
				sendMessage(c->getSock(), &pack1);
			}
			else
			{
				MessagePack pack1("Rename failed");
				sendMessage(c->getSock(), &pack1);
			}
			break;
		}
		case CMD_TEST:
		{
			//TestPack pack("djawodawdadjaiwodjoiawdjawjdawodijawidawjddwadwadada");
			//sendMessage(c->getSock(), &pack);
			break;
		}
		default:
		{
			std::cout << "Unresolved message:CMD=" << pk->CMD << " length=" << pk->LENGTH << std::endl;
			break;
		}
		}
	}

private:


	CLIENT* acceptClient()
	{
		SOCKET csock = INVALID_SOCKET;
		sockaddr_in csin = {};
		int sz = sizeof(csin);

#ifdef _WIN32
		csock = accept(ssock, (sockaddr*)&csin, &sz);
#else
		csock = accept(ssock, (sockaddr*)&csin, reinterpret_cast<socklen_t *>(&sz));
#endif  

		if (INVALID_SOCKET == csock)
		{
			std::cout << "Invalid client socket" << std::endl;
		}
		else
		{
			std::cout << "New client connection:" << csock << " IP:" << inet_ntoa(csin.sin_addr) << std::endl;
		}
		std::string username = "user" + std::to_string(csock);
		CLIENT* c = new CLIENT(csock, csin, csock, username);
		clients.push_back(c);
		return c;

	}


	int recvPack(CLIENT* c)
	{

		SOCKET csock = c->getSock();

		int len = recv(csock, recvBuf, RECV_BUF_SIZE, NULL);
		if (len <= 0)
		{
			std::cout << "client " << c->getUserName() << "(csock=" << csock << ")Disconnected" << std::endl;
			for (auto it = clients.begin(); it < clients.end(); it++)
			{
				if ((*it)->getSock() == c->getSock())
				{
					delete (*it);
					clients.erase(it);
					break;
				}
			}
			return CLIENT_DISCONNECT;
		}

		memcpy(c->getmsgBuf() + c->getLastBufPos(), recvBuf, len);
		c->setLastBufPos(c->getLastBufPos() + len);
		while (c->getLastBufPos() >= sizeof(Header))
		{
			Pack* pack = reinterpret_cast<Pack*>(c->getmsgBuf());
			if (c->getLastBufPos() >= pack->LENGTH)
			{
				int nSize = c->getLastBufPos() - pack->LENGTH;
				handleMessage(c, pack);
				memcpy(c->getmsgBuf(), c->getmsgBuf() + pack->LENGTH, nSize);
				c->setLastBufPos(nSize);
			}
			else
			{
				break;
			}
		}
		return CMD_SUCCESS;
	}
};




#endif 
