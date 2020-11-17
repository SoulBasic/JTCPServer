#pragma once
#ifndef _GLOBAL_HPP_
#define _GLOBAL_HPP_

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <string>
#include <memory>

#include "Pack.hpp"
#include "CELLTimestamp.hpp"

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

#define CLIENT_ERROR -1
#define CLIENT_SUCCESS 1

#define CLIENT_DISCONNECT -1

#define SEND_BUF_SIZE 20480
#define RECV_BUF_SIZE 4096
#define MSG_BUF_SIZE 20480
#define CLIENT_HEART_DEAD_TIME 60000
#define SERVER_SEND_TIME 200 


#endif