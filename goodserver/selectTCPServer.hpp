#ifndef _SELECTTCPServer_HPP_
#define _SELECTTCPServer_HPP_

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <atomic>

#include <string>
#include "../Pack.hpp"
#include "../CELLTimestamp.hpp"
#ifdef _WIN32
	#define FD_SETSIZE	512
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

#define CLIENT_DISCONNECT -1

#define RECV_BUF_SIZE 4096
#define MSG_BUF_SIZE 20480

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

class INetEvent
{
public:
	virtual void OnLeave(CLIENT* c) = 0;
};



class CellServer
{
private:
	SOCKET ssock;
	INetEvent* serverEvent;
	char recvBuf[RECV_BUF_SIZE] = {};
	std::unordered_map<SOCKET, CLIENT*> clients;
	std::vector<CLIENT*> clientsBuf;
	std::mutex mtx;
	std::thread* mainThread;
	fd_set fdRead;
	fd_set fdReadBak;
	SOCKET maxSocket;
	std::atomic<bool> fd_read_changed = false;
public:
	std::atomic<int> recvPackCount = 0;

	CellServer(SOCKET serverSock, INetEvent* evt) :ssock(serverSock), serverEvent(evt)
	{
	}
	
	~CellServer()
	{
		delete mainThread;
	}

	void start()
	{
		mainThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
		mainThread->detach();
	}

	inline size_t getClientCount() { return clients.size() + clientsBuf.size(); }

	void addClientToBuf(CLIENT* c)
	{
		std::lock_guard<std::mutex> lg(mtx);
		clientsBuf.push_back(c);

	}

	//判断服务器是否正常运行中
	inline bool active() { return ssock != INVALID_SOCKET; }

	bool OnRun()
	{
		while (active())
		{
			if (clientsBuf.size() > 0)
			{
				std::lock_guard<std::mutex> lock(mtx);
				for (auto c : clientsBuf)
				{
					clients.insert(std::make_pair(c->getSock(), c));
				}
				clientsBuf.clear();
				fd_read_changed = true;
			}
			if (clients.empty())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

			FD_ZERO(&fdRead);

			if (fd_read_changed)
			{
				maxSocket = clients.begin()->first;
				for (auto c : clients)
				{
					FD_SET(c.first, &fdRead);
					maxSocket = std::max(maxSocket, c.first);
				}
				memcpy(&fdReadBak,&fdRead, sizeof(fd_set));
				fd_read_changed = false;
			}
			else
			{
				memcpy(&fdRead, &fdReadBak, sizeof(fd_set));
			}


			timeval t = { 0,1 };
			int res = select(maxSocket + 1, &fdRead, nullptr, nullptr, &t);
			if (res < 0)
			{
				std::cout << "select模型未知错误，任务结束" << std::endl;
				ssock = INVALID_SOCKET;
				return false;
			}

#ifdef _WIN32
			for (int i = 0; i < fdRead.fd_count; i++)
			{
				auto it = clients.find(fdRead.fd_array[i]);
				if (it != clients.end())
				{
					if (CLIENT_DISCONNECT == recvPack(it->second))
					{
						std::cout << "客户" << it->second->getUserName() << "(csock=" << it->second->getUserName() << ")已断开连接" << std::endl;
						auto p = it->second;
						serverEvent->OnLeave(p);
						std::lock_guard<std::mutex> lg(mtx);
						clients.erase(it->first);
						fd_read_changed = true;
					}
				}
				else
				{
					std::cout << "奇怪的情况" << std::endl;
				}

			}
			
#else

			for (auto c : clients)
			{
				if (FD_ISSET(c.first, &fdRead))
				{
					if (CLIENT_DISCONNECT == recvPack(c.second))
					{
						std::cout << "客户" << c.second->getUserName() << "(csock=" << c.second->getUserName() << ")已断开连接" << std::endl;
						auto p = c.second;
						serverEvent->OnLeave(p);
						std::lock_guard<std::mutex> lg(mtx);
						clients.erase(c.first);
						fd_read_changed = true;
						break;
					}
				}
			}

#endif // _WIN32

			


		}
		
	}

	//接收并处理数据包
	int recvPack(CLIENT* c)
	{
		SOCKET csock = c->getSock();

		int len = recv(csock, recvBuf, RECV_BUF_SIZE, NULL);
		if (len <= 0)return CLIENT_DISCONNECT;

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

	//给客户端发消息
	template<typename PackType>
	int sendMessage(SOCKET csock, PackType* msg)
	{
		if (INVALID_SOCKET == ssock)
		{
			std::cout << "服务器套接字未初始化或无效" << std::endl;
			return CMD_ERROR;
		}
		int res = send(csock, (const char*)msg, sizeof(PackType), 0);
		if (SOCKET_ERROR == res)
		{
			std::cout << "发送数据包失败" << std::endl;
			return CMD_ERROR;
		}
		else
		{

			//std::cout << "发送数据包to(" << csock << " ) " << msg->CMD << " " << msg->LENGTH << std::endl;
		}
		return CMD_SUCCESS;
	}

	virtual void handleMessage(CLIENT* c, Pack* pk)
	{

		switch (pk->CMD)
		{
		case CMD_PRIVATEMESSAGE:
		{
			PrivateMessagePack* pack = static_cast<PrivateMessagePack*>(pk);
			std::cout << "转发私信 " << std::endl;
			std::string sourceName = "user";
			SOCKET target = 0;



			auto it = clients.begin();
			for (it; it != clients.end(); it++)
			{
				if ((*it).second->getUserName() == pack->targetName)
				{
					target = (*it).first;
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
				strcpy(pack1.message, "私信发送失败，目标用户不存在或已离线");
				sendMessage(c->getSock(), &pack1);
			}
			break;
		}
		case CMD_MESSAGE:
		{
			MessagePack* pack = static_cast<MessagePack*>(pk);
			std::cout << "从客户端(" << c->getSock() << ")收到的消息 :CMD=" << pack->CMD << " LENGTH=" << pack->LENGTH << " DATA=" << pack->message << std::endl;
			strcpy(pack->message, "消息已成功被服务器接收!");
			sendMessage(c->getSock(), pack);
			break;
		}
		case CMD_BROADCAST:
		{
			BroadcastPack* pack = static_cast<BroadcastPack*>(pk);
			std::cout << "广播消息" << std::endl;
			for (auto c1 : clients)
			{
				sendMessage(c1.first, pack);
			}
			break;
		}
		case CMD_NAME:
		{
			NamePack* pack = static_cast<NamePack*>(pk);
			std::string userName = "";
			std::string oldName = "";
			auto it = clients.begin();
			for (it; it != clients.end(); it++)
			{
				if (c->getSock() == (*it).first)
				{
					oldName = (*it).second->getUserName();
					(*it).second->setUserName(pack->name);
					userName = pack->name;
					break;
				}
			}
			if (it != clients.end())
			{

				std::cout << "用户" << oldName << "(" << c->getSock() << ")改名为" << userName << std::endl;
				userName = "已成功更改名称，现在的昵称为 " + userName;
				MessagePack pack1(userName.c_str());
				sendMessage(c->getSock(), &pack1);
			}
			else
			{
				MessagePack pack1("重命名失败");
				sendMessage(c->getSock(), &pack1);
			}
			break;
		}
		case CMD_TEST:
		{
			recvPackCount++;
			//TestPack pack("dwadawd");
			//sendMessage(c->getSock(), &pack);
			break;
		}
		default:
		{
			std::cout << "无法解析的消息:CMD=" << pk->CMD << " length=" << pk->LENGTH << std::endl;
			break;
		}
		}
	}

};


class TCPServer:public INetEvent
{
private:
	SOCKET ssock = INVALID_SOCKET;
	sockaddr_in ssin = {};
	fd_set fdRead;
	fd_set fdWrite;
	fd_set fdExp;
	CELLTimestamp timeStamp;
	std::mutex mtx_clients;
	std::unordered_map<SOCKET,CLIENT*> clients;
	std::vector<CellServer*> cellServers;
public:
	inline SOCKET getSocket() { return ssock; }
	inline sockaddr_in getSockaddr_in() { return ssin; }
	TCPServer(const TCPServer& other) = delete;
	const TCPServer& operator=(const TCPServer& other) = delete;
public:
	//初始化win环境
	explicit inline TCPServer(){}

	~TCPServer()
	{
		terminal();
	}

	//判断服务器是否正常运行中
	inline bool active() { return ssock != INVALID_SOCKET; }

	//初始化socket
	int initSocket()
	{
#ifdef _WIN32
		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (SOCKET_ERROR == WSAStartup(version, &data))
		{
			std::cout << "初始化Winsock环境失败" << std::endl;
		}
		else
		{
			std::cout << "已成功初始化Winsock环境!" << std::endl;
		}
#endif 
		ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == ssock)
		{
			std::cout << "初始化服务器失败" << std::endl;
			return CMD_ERROR;
		}
		else
		{
			std::cout << "服务器初始化成功!" << std::endl;
		}
		return CMD_SUCCESS;
	}

	//绑定并监听端口
	int bindServer(const char* ip, unsigned short port, int cellServerCount = 4)
	{
		if (INVALID_SOCKET == ssock)
		{
			initSocket();
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
			std::cout << "绑定端口失败" << std::endl;
			return CMD_ERROR;
		}
		else
		{
			std::cout << "端口绑定成功!" << std::endl;
		}

		if (SOCKET_ERROR == listen(ssock, 5))
		{
			std::cout << "侦听端口失败" << std::endl;
			return -1;
		}
		else
		{
			std::cout << "侦听端口成功!" << std::endl;
		}
		startCellServers(cellServerCount);
		return CMD_SUCCESS;
	}

	bool OnRun()
	{
		timeToMsg();
		if (!active())return false;
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		FD_SET(ssock, &fdRead);
		FD_SET(ssock, &fdWrite);
		FD_SET(ssock, &fdExp);
		timeval t = { 0,100 };
		int res = select(ssock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (res < 0)
		{
			std::cout << "select模型未知错误，任务结束" << std::endl;
			terminal();
			return false;
		}

		if (FD_ISSET(ssock, &fdRead))//新客户加入
		{
			acceptClient();
			FD_CLR(ssock, &fdRead);
		}

		return true;
	}

	//给客户端发消息
	template<typename PackType>
	int sendMessage(SOCKET csock, PackType* msg)
	{
		if (INVALID_SOCKET == ssock)
		{
			std::cout << "服务器套接字未初始化或无效" << std::endl;
			return CMD_ERROR;
		}
		int res = send(csock, (const char*)msg, sizeof(PackType), 0);
		if (SOCKET_ERROR == res)
		{
			std::cout << "发送数据包失败" << std::endl;
			return CMD_ERROR;
		}
		else
		{

			//std::cout << "发送数据包to(" << csock << " ) " << msg->CMD << " " << msg->LENGTH << std::endl;
		}
		return CMD_SUCCESS;
	}

	//关闭socket
	void terminal()
	{
		if (INVALID_SOCKET == ssock)return;
		for (auto s : cellServers)
		{
			delete s;
		}

#ifdef _WIN32
		for (auto c : clients)
		{
			closesocket(c.first);
			delete c.second;
		}

		closesocket(ssock);
		WSACleanup();
#else //Linux
		for (auto c : clients)
		{
			close(c.first);
			delete c.second;
		}
		close(ssock);
#endif 
		ssock = INVALID_SOCKET;
		clients.clear();
	}



private:
	void timeToMsg()
	{
		auto t1 = timeStamp.getElapsedTimeInSec();
		if (t1 >= 1.0)
		{
			int count = 0;
			for (auto s : cellServers)
			{
				count += s->recvPackCount;
				s->recvPackCount = 0;
			}
			//if (count == 0)return;
			float speed = static_cast<float>(count * sizeof(TestPack)) / 1048576.0f / t1;
			std::cout << "" << t1 << " " << clients.size() << "个客户端收到了" << count << "个包，速度" << speed << "MB/s" << std::endl;
			for (auto cs : cellServers)
			{
				std::cout << cs->getClientCount() << " ";
			}
			std::cout << "\n";
			timeStamp.update();
		}
	}
	//接收连接的客户端
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
			std::cout << "无效的客户端套接字" << std::endl;
		}
		else
		{
			//std::cout << "新客户端连接:" << csock << " IP:" << inet_ntoa(csin.sin_addr) << std::endl;
			std::string username = "user" + std::to_string(csock);
			addClientToCellServer(new CLIENT(csock, csin, csock, username));			
		}

		return nullptr;

	}

	void addClientToCellServer(CLIENT* c)
	{
		auto ms = cellServers[0];
		for (auto s : cellServers)
		{
			if (s->getClientCount() < ms->getClientCount())
			{
				ms = s;
			}
		}
		ms->addClientToBuf(c);
		std::lock_guard<std::mutex> lg(mtx_clients);
		clients.insert(std::make_pair(c->getSock(), c));
	}

	void startCellServers(int cellServerCount)
	{
		for (int i = 0; i < cellServerCount; i++)
		{
			CellServer* s = new CellServer(ssock,this);
			s->start();
			cellServers.push_back(s);
		}
	}

	virtual void OnLeave(CLIENT* c)
	{
		std::lock_guard<std::mutex> lg(mtx_clients);
		auto it = clients.find(c->getSock());
		if (it != clients.end())
		{
			auto p = (*it).second;
			clients.erase(it);
			delete (p);
		}
	}

};




#endif 
