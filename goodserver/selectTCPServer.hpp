#ifndef _SELECTTCPServer_HPP_
#define _SELECTTCPServer_HPP_

#include "Client.hpp"
#include "CellServer.hpp"
#include "INetEvent.hpp"


class TCPServer : public INetEvent
{
private:
	std::unordered_map<SOCKET, std::shared_ptr<CLIENT>> clients;
	std::vector<CellServer*> cellServers;
	sockaddr_in ssin = {};
	fd_set fdRead;
	fd_set fdWrite;
	fd_set fdExp;
	CELLTimestamp timeStamp;
	std::mutex mtx_clients;
	SOCKET ssock = INVALID_SOCKET;
public:
	inline SOCKET getSocket() { return ssock; }
	inline sockaddr_in getSockaddr_in() { return ssin; }
	TCPServer(const TCPServer& other) = delete;
	const TCPServer& operator=(const TCPServer& other) = delete;
public:
	//初始化win环境
	explicit inline TCPServer() {}

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


	//关闭socket
	void terminal()
	{
		if (INVALID_SOCKET == ssock)return;
		for (auto s : cellServers)
		{
			delete s;
		}
		std::cout << "terminal" << std::endl;
#ifdef _WIN32
		
		closesocket(ssock);
		WSACleanup();
#else //Linux
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
			//for (auto cs : cellServers)
			//{
			//	std::cout << cs->getClientCount() << " ";
			//}
			//std::cout << "\n";
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
			return nullptr;
			//std::cout << "无效的客户端套接字" << std::endl;
		}
		else
		{
			//std::cout << "新客户端连接:" << csock << " IP:" << inet_ntoa(csin.sin_addr) << std::endl;
			std::string username = "user" + std::to_string(csock);
			std::shared_ptr<CLIENT> c(new CLIENT(csock, csin, csock, username));
			addClientToCellServer(c);
		}

		return nullptr;

	}

	void addClientToCellServer(std::shared_ptr<CLIENT> c)
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
			CellServer* s = new CellServer(ssock, this);
			s->start();
			cellServers.push_back(s);
		}
	}

	virtual void OnLeave(std::shared_ptr<CLIENT> c)
	{
		std::lock_guard<std::mutex> lg(mtx_clients);
		auto it = clients.find(c->getSock());
		if (it != clients.end())
		{
			c->setAlive(false);
			clients.erase(it);
		}
	}

};


#endif 
