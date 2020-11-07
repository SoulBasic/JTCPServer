#ifndef _Pack_HPP_
#define _Pack_HPP_
#include <iostream>
#include <string>

#define CMD_LOGIN 101
#define CMD_MESSAGE 102
#define CMD_PRIVATEMESSAGE 103


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
		strcpy_s(userName, "\0");
		strcpy_s(passWord, "\0");
		LENGTH = sizeof(LoginPack);
		CMD = CMD_LOGIN;
	}
	LoginPack(char un[32],char pw[32])
	{
		strcpy_s(userName, un);
		strcpy_s(passWord, pw);
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
		strcpy_s(message, "\0");
		LENGTH = sizeof(MessagePack);
		CMD = CMD_MESSAGE;
	}
	MessagePack(const char* msg)
	{
		strcpy_s(message, msg);
		LENGTH = sizeof(MessagePack);
		CMD = CMD_MESSAGE;
	}
};

class PrivateMessagePack : public Header
{
public:
	int targetUID;
	char message[4084];
	PrivateMessagePack() :targetUID(0)
	{
		strcpy_s(message, "\0");
		LENGTH = sizeof(PrivateMessagePack);
		CMD = CMD_PRIVATEMESSAGE;
	}
};


#endif // !_Pack_HPP_

