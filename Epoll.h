#ifndef __EPOLL_H__
#define __EPOLL_H__

#include"Header.h"
#include"Encry.h"

class IgnoreSigPipe
{
public:
	IgnoreSigPipe()
	{
		::signal(SIGPIPE, SIG_IGN);
	}
};

static IgnoreSigPipe initPIPE_IGN;

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

	//epoll_ctl事件
	void OPEvent(int fd, int events, int how)
	{
		struct epoll_event ev;
		ev.events = events;
		ev.data.fd = fd;
		if (epoll_ctl(_eventfd, how, fd, &ev) < 0 )
		{
			ErrorDebug("epoll_ctl.fd:&d + how:%d", fd, how);
		}
	}

	//设置非阻塞
	void SetNonblocking(int fd)
	{
		int flags, s;
		flags = fcntl(fd, F_GETFL, 0);
		if (flags == -1)
			ErrorDebug("SetNonblocking:F_GETFL");
		flags |= O_NONBLOCK;
		s = fcntl(fd, F_SETFL, flags);
		if (s == -1)
			ErrorDebug("SetNonblocking : F_SETFL");
	}
	
	//启动服务，开始监听
	void Start();

	//事件循环
	void EventLoop();

	//纯虚函数，子类必须重写
	virtual void ConnectEventHandle(int fd) = 0;
	virtual void ReadEventHandle(int fd) = 0;
	virtual void WriteEventHandle(int fd);

	//状态
	enum Socks5State
	{
		AUTH,            //身份认证
		ESTABLISHMENT,	 //建立连接
		FORWARDING,		 //转发
	};

	//通道    --保存未完全接受的数据
	struct Channel
	{
		int _fd;		//描述符
		int _event;	    //事件
		string _buffer; //缓冲区

		Channel()
			: _fd(-1)
			, _event(0)
		{}
	};

	//连接
	struct Connect
	{
		Socks5State _state;		 //连接的状态
		Channel _clientChannel;  //客户端通道
		Channel _serverChannel;  //服务器通道
		int _ref;

		Connect()
			:_state(AUTH)
			, _ref(0)
		{}
	};

	void SendInLoop(int fd, const char* buf, int len);
	void Forwarding(Channel* clientChannel, Channel* serverChannel, bool sendencry, bool recvdecrypt);
	void RemoveConnect(int fd);

private:
	//防拷贝
	EpollServer(const EpollServer&);
	EpollServer& operator=(const EpollServer&);

protected:
	int _listenfd;   //监听套接字
	int _port;       //服务端口

	int _eventfd;    //事件描述符
	static const size_t _MAX_EVENT; //最大事件数量

	map<int, Connect*> _fdConnectMap;  //fd映射连接的map容器
};

#endif //__EPOLL_H__