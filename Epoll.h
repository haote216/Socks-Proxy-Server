#ifndef __EPOLL_H__
#define __EPOLL_H__

#include"Header.h"

class EpollServer
{
public:
	EpollServer(int port)
		: _listenfd(-1)
		, _port(port)
		, _eventfd(-1)
	{}

	virtual ~EpollServer()
	{
		if (_listenfd != -1)
			close(_listenfd);
	}
	
	//启动服务，开始监听
	void Start();

	//事件循环
	void EventLoop();

	//纯虚函数，子类必须重写
	virtual void ConnectEventHandle(int fd) = 0;
	virtual void ReadEventHandle(int fd) = 0;
	virtual void WriteEventHandle(int fd) = 0;

private:
	//防拷贝
	EpollServer(const EpollServer&);
	EpollServer& operator=(const EpollServer&);

protected:
	int _listenfd;   //监听套接字
	int _port;       //服务端口

	int _eventfd;    //事件描述符
	static const size_t _MAX_EVENT; //最大事件数量
};

#endif //__EPOLL_H__