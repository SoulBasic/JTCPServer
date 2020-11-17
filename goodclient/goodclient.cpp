

#include "selectTCPClient.hpp"
#include <memory>
const int clientNum = 1000;
const int threadNum = 10;
bool running = false;
TCPClient* clients[clientNum];

void cmdThread(TCPClient* c)
{
	std::string cmd = "";
	while (running)
	{
		std::cin >> cmd;
		if ("quit" == cmd)
		{
			running = false;
			break;
		}
		else if (cmd == "cpm")
		{
			PrivateMessagePack pack;
			std::cout << "请输入私信目标昵称：";
			std::cin >> cmd;
			strcpy(pack.targetName, cmd.c_str());
			std::cout << "请输入私信内容：";
			std::cin >> cmd;
			strcpy(pack.message, cmd.c_str());
			c->sendMessage(&pack);
		}
		else if (cmd == "bc")
		{
			BroadcastPack pack;
			std::cout << "请输入广播内容：";
			std::cin >> cmd;
			strcpy(pack.message, cmd.c_str());
			c->sendMessage(&pack);
		}
		else if (cmd == "name")
		{
			NamePack pack;
			std::cout << "请输入您的昵称 ：";
			std::cin >> cmd;
			strcpy(pack.name, cmd.c_str());
			c->sendMessage(&pack);
		}
		else if(cmd == "msg")
		{
			std::cout << "请输入消息内容：";
			std::cin >> cmd;
			MessagePack pack;
			strcpy(pack.message, cmd.c_str());
			c->sendMessage(&pack);
		}
		
		else
		{
			std::cout << "未定义的命令" << std::endl;
		}

	}
}


void recvThread(TCPClient* c)
{
	while (c->active())
	{
		c->onRun();
	}
	running = false;
}

void sendThread(int id)
{
	int c = clientNum / threadNum;
	int begin = (id - 1)*c;
	int end = id * c;
	
	for (int i = begin; i < end; i++)
	{
		if (!running)return;
		clients[i] = new TCPClient("192.168.199.132",2324);
	}
	for (int i = begin; i < end; i++)
	{
		clients[i]->connectServer();
	}

	TestPack pack("123213213");
	HeartPack hpack;
	while (running)
	{
		time_t nowTime = NOWTIME_MILLI;                     
		for (int i = begin; i < end; i++)
		{
			if (INVALID_SOCKET != clients[i]->getCsock())
			{
				clients[i]->onRun();
				if (nowTime - clients[i]->getHeart() >= CLIENT_HEART_DEAD_TIME/2)
				{
					//clients[i]->sendMessage(&hpack);
					//clients[i]->setHeart(nowTime);
				}
				clients[i]->sendMessage(&pack);
			}
		}
	}

	for (int i = begin; i < end; i++)
	{
		clients[i]->terminal();
		delete clients[i];
	}
}


int main()
{
	//TCPClient c("127.0.0.1", 2324);
	//c.initSocket();
	//if (CLIENT_ERROR == c.connectServer()) return -1;
	running = true;
	std::cout << "开始！" << std::endl;
	//std::thread tcmd(cmdThread,&c);
	//tcmd.detach();
	//std::thread trecv(recvThread,&c);
	//trecv.detach();
	for (int i = 1; i <= threadNum; i++)
	{
		std::thread t1(sendThread, i);
		t1.detach();
	}

	while (running)
	{
		Sleep(500);
	}

	std::cout << "程序结束" << std::endl;
	::system("pause");
	return 0;
}
