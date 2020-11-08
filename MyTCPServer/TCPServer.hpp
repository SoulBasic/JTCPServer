#ifndef _TCPServer_HPP_
#define _TCPServer_HPP_

#include <iostream>
#include <tuple>
#include <vector>
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

#define CMD_ERROR 0
#define CMD_SUCCESS 1


typedef std::tuple<SOCKET, sockaddr_in> CLIENT;





class TCPServer
{
private:
	SOCKET ssock;
	sockaddr_in ssin;
	//std::vector<CLIENT> clients;
public:
	inline SOCKET getSocket() { return ssock; }
	inline sockaddr_in getSockaddr_in() { return ssin; }
	//inline std::vector<CLIENT>* getClients() { return &clients; }
	TCPServer(const TCPServer& other) = delete;
	const TCPServer& operator=(const TCPServer& other) = delete;
public:
	//初始化win环境
	explicit inline TCPServer()
	{
		ssock = INVALID_SOCKET;
		ssin = {};
	}

	~TCPServer()
	{
		terminal();
	}

	//初始化socket
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
			std::cout << "Successfully initialized Winsock environment!" << std::endl;
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
			std::cout << "Initialization of server succeeded!" << std::endl;
		}
		return CMD_SUCCESS;
	}

	//绑定并监听端口
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

	//关闭socket
	void terminal()
	{
		if (INVALID_SOCKET == ssock)return;
#ifdef _WIN32
		closesocket(ssock);
		WSACleanup();
#else //Linux
		close(ssock);
#endif 
		ssock = INVALID_SOCKET;
	}

	//接收连接的客户端
	CLIENT acceptClient()
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
			std::cout << "New client connected SOCKET:"<< csock << " IP:" << inet_ntoa(csin.sin_addr) << std::endl;
		}
		return std::make_tuple(csock, csin);

	}

	//给客户端发消息
	template<typename PackType>
	int sendMessage(SOCKET csock, PackType& msg)
	{
		if (INVALID_SOCKET == ssock)
		{
			std::cout << "The server socket is not initialized or invalid" << std::endl;
			return CMD_ERROR;
		}
		int res = send(csock, (const char*)&msg, sizeof(msg), 0);
		if (SOCKET_ERROR == res)
		{
			std::cout << "Sending packet failed" << std::endl;
			return CMD_ERROR;
		}
		return CMD_SUCCESS;
	}

	//接收消息
	template<typename PackType>
	PackType receive(SOCKET csock)
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

};




#endif // !_TCPClient_HPP_

