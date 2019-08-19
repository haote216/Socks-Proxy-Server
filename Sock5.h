#ifndef __SOCK5_H__
#define __SOCK5_H__

#include"Epoll.h"

class Sock5Server : public EpollServer
{
public:
	Sock5Server(int port = 8001)
		:EpollServer(port)
	{}

	int AuthHandle(int fd);
	int EstablishmentHandle(int fd);

	virtual void ConnectEventHandle(int fd);
	virtual void ReadEventHandle(int fd);

protected:
	struct sockaddr_in _socks5addr;    //socks5  server addr
};

#endif //__SOCK5_H__