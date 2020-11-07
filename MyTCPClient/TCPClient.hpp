#ifndef _TCPClient_HPP_
#define _TCPClient_HPP_

#include <iostream>
#include "../Pack.hpp"
#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#else//Linux
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif 

#define CLIENT_ERROR -1
#define CLIENT_SUCCESS 1



class TCPClient
{
public:
	TCPClient()
	{
		csock = INVALID_SOCKET;
		ssin = {};
#ifdef _WIN32
		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (SOCKET_ERROR == WSAStartup(version, &data))
		{
			std::cout << "初始化WINSOCK环境失败" << std::endl;
		}
		else
		{
			std::cout << "初始化WINSOCK环境成功!" << std::endl;
		}
#endif 
	}
	~TCPClient()
	{
		terminal();
#ifdef _WIN32
		WSACleanup();
#endif 
	}

	int initSocket()
	{
		csock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == csock)
		{
			std::cout << "初始化客户端失败" << std::endl;
			return CLIENT_ERROR;
		}
		else
		{
			std::cout << "初始化客户端成功!" << std::endl;
		}
		return CLIENT_SUCCESS;
	}

	int connectServer(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == csock)
		{
			std::cout << "套接字未初始化或无效" << std::endl;
			return CLIENT_ERROR;
		}
		ssin.sin_family = AF_INET;
		ssin.sin_port = htons(port);
#ifdef _WIN32
		ssin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		ssin.sin_addr.s_addr = inet_addr(ip);
#endif 

		int res = connect(csock, (sockaddr*)&ssin, sizeof(ssin));
		if (SOCKET_ERROR == res)
		{
			std::cout << "连接服务器失败" << std::endl;
			return CLIENT_ERROR;
		}
		else
		{
			std::cout << "连接服务器成功!" << std::endl;
		}
		return CLIENT_SUCCESS;
	}

	void terminal()
	{
		if (INVALID_SOCKET == csock)return;
#ifdef _WIN32
		closesocket(csock);
#else //Linux
		close(csock);
#endif 
		csock = INVALID_SOCKET;
	}

	template<typename PackType>
	int sendMessage(PackType& msg)
	{
		if (INVALID_SOCKET == csock)
		{
			std::cout << "套接字未初始化或无效" << std::endl;
			return CLIENT_ERROR;
		}
		int res = send(csock, (const char*)&msg, sizeof(msg), 0);
		if (SOCKET_ERROR == res)
		{
			std::cout << "发送数据包失败" << std::endl;
			return CLIENT_ERROR;
		}
		return CLIENT_SUCCESS;
	}


	template<typename PackType>
	PackType receive()
	{
		PackType buf;
		if (sizeof(PackType) == sizeof(Header))
		{
			recv(csock, (char*)&buf, sizeof(buf), 0);
		}
		else
		{
			recv(csock, (char*)&buf + sizeof(Header), sizeof(buf) - sizeof(Header), 0);
		}
		return buf;
	}

private:
	SOCKET csock;
	sockaddr_in ssin;

};




#endif // !_TCPClient_HPP_

