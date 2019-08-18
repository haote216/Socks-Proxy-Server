#ifndef __SOCK5_H__
#define __SOCK5_H__

#include"Epoll.h"

class Sock5Server : public EpollServer
{
public:
	Sock5Server(const char* socks5ServerIp, int socks5ServerPort, int selfPort = 8000)
		:EpollServer(selfPort)
	{
		memset(&_socks5addr, 0, sizeof(struct sockaddr_in));
		_socks5addr.sin_family = AF_INET;
		_socks5addr.sin_port = htons(socks5ServerPort);
		_socks5addr.sin_addr.s_addr = inet_addr(socks5ServerIp);
	}

	int AuthHandle(int fd);
	int EstablishmentHandle(int fd);
	void Forwarding(Channel* clientChannel, Channel* serverChannel);
	void RemoveConnect(int fd);

	virtual void ConnectEventHandle(int fd);
	virtual void ReadEventHandle(int fd);
	virtual void WriteEventHandle(int fd);

protected:
	struct sockaddr_in _socks5addr;    //socks5  server addr
};

#endif //__SOCK5_H__