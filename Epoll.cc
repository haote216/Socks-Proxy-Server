#include"Epoll.h"

//启动服务，开始监听
void EpollServer::Start()
{
	int _listenfd = socket(AF_INET,SOCK_STREAM,0);
	if (_listen == -1)
	{
		ErrorLog("create socket error");
		return;
	}

	struct sockaddr_in addr;
	memeset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(_port);

	if (bind(_listenfd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
	{
		ErrorLog("bind socket error");
		return;
	}
	if (listen(_listenfd, 100000) == -1)
	{
		ErrorLog("listen socket error");
		return;
	}

	//event
	_eventfd = epoll_create(_MAX_EVENT);
	if (_eventfd == -1)
	{
		ErrorLog("epoll_create error");
		return;
	}

	//添加套接字
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = _listenfd;
	if (epoll_ctl(_eventfd, EPOLL_CTL_ADD, _listenfd, &event) == -1)
	{
		ErrorLog("EPOLL_CTL_ADD _listenfd EPOLLIN error");
		return;
	}

	//epoll开始监听
	EventLoop();
}

//事件循环
void EpollServer::EventLoop()
{
	//事件数组
	struct epoll_event events[_MAX_EVENT]; 

	while (1)
	{
		int size = epoll_wait(_eventfd, events, _MAX_EVENT, -1);

		if (size == -1)
		{
			ErrorLog("epoll_wait error");
		}

		for (size_t i = 0; i < size; ++i)
		{
			if (events[i].data.fd == _listenfd)
			{
				struct sockaddr addr;
				socklen_t * len;
				int connectfd = accept(_listenfd, &addr, &len);
				if (connectfd == -1)
				{
					ErrorLog("accept error");
				}

				TranceLog("client connect");

				ConnectEventHandle(connectfd);
			}
			else if (events[i].events & EPOLLIN)
			{
				ReadEventHandle(events[i].data.fd);
			}
			else if (events[i].events & EPOLLOUT)
			{
				WriteEventHandle(events[i].data.fd);
			}
			else
			{
				ErrorLog("events error");
			}
	}
	

	}
}