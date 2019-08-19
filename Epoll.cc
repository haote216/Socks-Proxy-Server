#include"Epoll.h"
const size_t EpollServer::_MAX_EVENT = 100000;

//启动服务，开始监听
void EpollServer::Start()
{
	_listenfd = socket(AF_INET,SOCK_STREAM,0);
	if (_listenfd == -1)
	{
		ErrorDebug("create socket error");
		return;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	//监听本机的any网卡
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(_port);

	if (bind(_listenfd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
	{
		ErrorDebug("bind socket error");
		return;
	}
	if (listen(_listenfd, 100000) == -1)
	{
		ErrorDebug("listen socket error");
		return;
	}

	//event
	_eventfd = epoll_create(_MAX_EVENT);
	if (_eventfd == -1)
	{
		ErrorDebug("epoll_create error");
		return;
	}

	//添加套接字
	OPEvent(_listenfd, EPOLLIN, EPOLL_CTL_ADD);

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
			ErrorDebug("epoll_wait error");
		}
		for (int i = 0; i < size; ++i)
		{
			if (events[i].data.fd == _listenfd)
			{

				struct sockaddr_in addr;
				socklen_t len = sizeof(addr);
				int connectfd = accept(_listenfd,(struct sockaddr*)&addr, &len);
				if (connectfd == -1)
				{
					ErrorDebug("accept error");
					//continue;
				}
				//TraceDebug("client connect");
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
				ErrorDebug("events error");
			}
		}	
	}
}

void EpollServer::SendInLoop(int fd, const char* buf, int len)
{
	int slen = send(fd, buf, len, 0);
	if (slen < 0)
	{
		ErrorDebug("send: to %d", fd);
	}
	else if (slen < len)
	{
		TraceDebug("recv %d bytes, send %d bytes, left %d send in loop", len, slen, len - slen);
		map<int, Connect*>::iterator it = _fdConnectMap.find(fd);
		if (it != _fdConnectMap.end())
		{
			Connect*  con = it->second;
			Channel* channel = &con->_clientChannel;
			if (fd == con->_serverChannel._fd)
				channel = &con->_serverChannel;

			int events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
			OPEvent(fd, events, EPOLL_CTL_MOD);

			channel->_buffer.append(buf + slen);
		}
	}
}

void EpollServer::Forwarding(Channel* clientChannel, Channel* serverChannel, bool sendencry, bool recvdecrypt)
{
	char buf[4096];
	int rlen = recv(clientChannel->_fd, buf, 4096, 0);
	if (rlen < 0)
	{
		ErrorDebug("recv: %d", clientChannel->_fd);
	}
	else if (rlen == 0)
	{
		//client channel发起关闭
		shutdown(serverChannel->_fd, SHUT_WR);
		RemoveConnect(clientChannel->_fd);
	}
	else
	{
		if (recvdecrypt)
		{
			Decrypt(buf, rlen);
		}

		if (sendencry)
		{
			Encry(buf, rlen);
		}
		buf[rlen] = '\0';
		SendInLoop(serverChannel->_fd, buf, rlen);
	}
}

void EpollServer::RemoveConnect(int fd)
{
	OPEvent(fd, 0, EPOLL_CTL_DEL);
	map<int, Connect*>::iterator it = _fdConnectMap.find(fd);
	if (it != _fdConnectMap.end())
	{
		Connect* con = it->second;
		if (--con->_ref == 0)
		{
			delete con;
			_fdConnectMap.erase(it);
		}
	}
	else
	{
		assert(false);
	}
}

void EpollServer::WriteEventHandle(int fd)
{
	map<int, Connect*>::iterator it = _fdConnectMap.find(fd);
	if (it != _fdConnectMap.end())
	{
		Connect* con = it->second;
		Channel* channel = &con->_clientChannel;
		if (fd == con->_serverChannel._fd)
		{
			channel = &con->_serverChannel;
		}

		string buff;
		buff.swap(channel->_buffer);
		SendInLoop(fd, buff.c_str(), buff.size());
	}
	else
	{
		assert(fd);
	}
}