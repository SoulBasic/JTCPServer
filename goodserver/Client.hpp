#pragma once

#ifndef _CLIENT_HPP_
#define _CLIENT_HPP_

#include "../Global.hpp"

class CLIENT
{
public:
	CLIENT(SOCKET csock, sockaddr_in csin, int userid, std::string username)
		:sock(csock), sin(csin), lastBufPos(0), userID(userid), userName(username)
	{
		dtHeart = NOWTIME_MILLI;
	}
	~CLIENT()
	{
		if (INVALID_SOCKET != sock)
		{
#ifdef _WIN32
			closesocket(sock);
#else //Linux
			close(sock);
#endif 
			sock = INVALID_SOCKET;
		}

	}
	inline SOCKET getSock() { return sock; }
	inline sockaddr_in getSin() { return sin; }
	inline char* getmsgBuf() { return msgBuf; }
	inline int getLastBufPos() { return lastBufPos; }
	inline void setLastBufPos(int val) { lastBufPos = val; }
	inline int getUserID() { return userID; }
	inline std::string getUserName() { return userName; }
	inline void setUserName(std::string username) { userName = username; }
	inline void resetHeart(){ dtHeart = NOWTIME_MILLI; }
	bool checkHeart(time_t dt) 
	{
		if (dt - dtHeart >= CLIENT_HEART_DEAD_TIME)
		{
			std::cout << "心跳检测：客户端离开" << std::endl;
			return false;
		}
		return true; 
	}
private:
	SOCKET sock;
	sockaddr_in sin;
	char msgBuf[MSG_BUF_SIZE] = {};
	int lastBufPos;
	int userID;
	std::string userName;
	time_t dtHeart;
};




#endif