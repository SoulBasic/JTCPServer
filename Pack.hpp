#ifndef _Pack_HPP_
#define _Pack_HPP_
#include <iostream>
#include <cstring>

#define CMD_LOGIN 101
#define CMD_MESSAGE 102
#define CMD_PRIVATEMESSAGE 103
#define CMD_BROADCAST 104
#define CMD_NAME 105


class Header
{
public:
	int LENGTH;
	int CMD;
	Header() :LENGTH(0), CMD(0){}
	Header(int len, int cmd) :LENGTH(len),CMD(cmd){}
	virtual ~Header(){}
};



class LoginPack : public Header
{
public:
	char userName[32];
	char passWord[32];
	LoginPack()
	{
		strcpy(userName, "\0");
		strcpy(passWord, "\0");
		LENGTH = sizeof(LoginPack);
		CMD = CMD_LOGIN;
	}
	LoginPack(char un[32],char pw[32])
	{
		strcpy(userName, un);
		strcpy(passWord, pw);
		LENGTH = sizeof(LoginPack);
		CMD = CMD_LOGIN;
	}
};


class MessagePack : public Header
{
public:
	char message[1012];
	MessagePack()
	{
		strcpy(message, "\0");
		LENGTH = sizeof(MessagePack);
		CMD = CMD_MESSAGE;
	}
	MessagePack(const char* msg)
	{
		strcpy(message, msg);
		LENGTH = sizeof(MessagePack);
		CMD = CMD_MESSAGE;
	}
};

class PrivateMessagePack : public Header
{
public:
	char targetName[32];
	char message[4052];
	PrivateMessagePack() 
	{
		strcpy(targetName, "\0");
		strcpy(message, "\0");
		LENGTH = sizeof(PrivateMessagePack);
		CMD = CMD_PRIVATEMESSAGE;
	}
};


class BroadcastPack : public Header
{
public:
	char message[1012];
	BroadcastPack()
	{
		strcpy(message, "\0");
		LENGTH = sizeof(BroadcastPack);
		CMD = CMD_BROADCAST;
	}
	BroadcastPack(const char* msg)
	{
		strcpy(message, msg);
		LENGTH = sizeof(BroadcastPack);
		CMD = CMD_BROADCAST;
	}
};

class NamePack : public Header
{
public:
	char name[32];
	NamePack()
	{
		LENGTH = sizeof(NamePack);
		CMD = CMD_NAME;
	}
};

#endif // !_Pack_HPP_

