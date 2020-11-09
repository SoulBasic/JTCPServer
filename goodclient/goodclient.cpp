// goodclient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include "selectTCPClient.hpp"
#include <string>
#include <atomic>
#include <bitset>


const int clientNum = 1000;
const int threadNum = 2;
bool running = false;


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

void sendThread(TCPClient* c)
{
	while (running)
	{
		TestPack pack("这等级的哇大无多安慰大武当阿达啊我打完的啊哦的旧爱为大我觉得加我激动阿达基调哦对马咯打我一等奖我安慰奖do我案件我一到家");
		c->sendMessage(&pack);
	}

}

int main()
{
	TCPClient c("149.28.24.77", 2324);
	c.initSocket();
	if (CLIENT_ERROR == c.connectServer()) return -1;

	running = true;
	std::thread tcmd(cmdThread,&c);
	tcmd.detach();
	std::thread trecv(recvThread,&c);
	trecv.detach();

	for (int i = 1; i <= threadNum; i++)
	{
		std::thread t1(sendThread, &c);
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
