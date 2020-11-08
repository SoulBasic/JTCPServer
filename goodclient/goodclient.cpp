// goodclient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include "selectTCPClient.hpp"
#include <string>
#include <atomic>
using namespace std;

const int clientNum = 1000;
const int threadNum = 1;
bool running = false;


void cmdThread()
{
	TCPClient c("127.0.0.1", 2324);
	c.initSocket();
	c.connectServer();
	string cmd = "";
	while (true)
	{
		cin >> cmd;
		if ("quit" == cmd)
		{
			running = false;
			break;
		}
		else if ("send" == cmd)
		{
			MessagePack pack;
			strcpy(pack.message, "hhhha");
			c.sendMessage(pack);
		}
		else
		{
			
			cout << "未定义的命令" << endl;
		}
		
	}
}

void sendThread(int id)
{
	TCPClient c("127.0.0.1", 2324);
	c.initSocket();
	c.connectServer();
	MessagePack pack;
	strcpy(pack.message, "hhhha");
	c.sendMessage(pack);
}


int main()
{
	thread tcmd(cmdThread);
	tcmd.detach();
	
	running = true;
	for (int i = 1; i <= threadNum; i++)
	{
		thread t1(sendThread, i);
		t1.detach();
	}
	
	while (running)
	{
		Sleep(500);
	}

	cout << "程序结束" << endl;
}
