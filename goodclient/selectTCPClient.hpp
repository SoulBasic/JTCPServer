#ifndef _SELECTTCPClient_HPP_
#define _SELECTTCPClient_HPP_

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
private:
	SOCKET csock;
	sockaddr_in ssin;
public:
	void setSsin(const char* ip, unsigned short port)
	{
		ssin = {};
		ssin.sin_family = AF_INET;
		ssin.sin_port = htons(port);
#ifdef _WIN32
		ssin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		ssin.sin_addr.s_addr = inet_addr(ip);
#endif 
	}
public:
	TCPClient()
	{
		csock = INVALID_SOCKET;
		ssin = {};
	}
	TCPClient(const char* ip, unsigned short port)
	{
		csock = INVALID_SOCKET;
		ssin = {};
		ssin.sin_family = AF_INET;
		ssin.sin_port = htons(port);
#ifdef _WIN32
		ssin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		ssin.sin_addr.s_addr = inet_addr(ip);
#endif 
	}
	~TCPClient()
	{
		terminal();
	}

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
		csock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == csock)
		{
			std::cout << "Failed to initialize client" << std::endl;
			return CLIENT_ERROR;
		}
		else
		{
			std::cout << "Client initialization successful!" << std::endl;
		}
		return CLIENT_SUCCESS;
	}

	int connectServer()
	{
		if (INVALID_SOCKET == csock)
		{
			std::cout << "Socket is not initialized or invalid" << std::endl;
			return CLIENT_ERROR;
		}
		int res = connect(csock, (sockaddr*)&ssin, sizeof(ssin));
		if (SOCKET_ERROR == res)
		{
			std::cout << "Failed to connect to the server" << std::endl;
			return CLIENT_ERROR;
		}
		else
		{
			std::cout << "Successfully connected to the server!" << std::endl;
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
#ifdef _WIN32
		WSACleanup();
#endif 
	}

	template<typename PackType>
	int sendMessage(PackType& msg)
	{
		if (INVALID_SOCKET == csock)
		{
			std::cout << "Socket is not initialized or invalid" << std::endl;
			return CLIENT_ERROR;
		}
		int res = send(csock, (const char*)&msg, sizeof(msg), 0);
		if (SOCKET_ERROR == res)
		{
			std::cout << "Sending packet failed" << std::endl;
			return CLIENT_ERROR;
		}
		else
		{
			std::cout << "send successed" << std::endl;
		}
		return CLIENT_SUCCESS;
	}


	template<typename PackType>
	bool receive(PackType& buf)
	{
		auto sz = sizeof(Header);
		if (sizeof(PackType) == sizeof(Header))
		{
			if (recv(csock, (char*)&buf, sizeof(buf), 0) <= 0)
			{
				return false;
			}
		}
		else
		{
			if (recv(csock, (char*)&buf + sizeof(Header), sizeof(buf) - sizeof(Header), 0) <= 0)
			{
				return false;
			}
		}
		return true;
		
	}



};




#endif // !_TCPClient_HPP_

