
#include <iostream>
#include "selectTCPServer.hpp"
#include <unordered_map>


//分割字符串
std::vector<std::string> split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;//扩展字符串以方便操作
	int size = str.size();
	for (int i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

//unordered_map value找key
template<typename keyT, typename valueT>
bool getKey(std::unordered_map<keyT, valueT>& map, valueT val, keyT& key)
{
	for (auto& i : map)
	{
		if (i.second == val)
		{
			key = i.first;
			return true;
		}
	}
	return false;
}


int main()
{
	TCPServer server;
	server.initSocket();
	server.bindServer("127.0.0.1", 2324);

	while (server.active())
	{
		server.OnRun();
	}
}
