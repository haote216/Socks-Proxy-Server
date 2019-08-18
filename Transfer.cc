//#include"Transfer.h"
//
//void TransferServer::ConnectEventHandle(int connectfd)
//{
//
//	//添加connectfd到epoll，监听读事件
//	SetNonblocking(connectfd);
//	OPEvent(connectfd, EPOLLIN, EPOLL_CTL_ADD);
//
//	//创建连接及客户端服务通道
//	Connect* con = new Connect;
//	con->_state = AUTH;
//	con->_clientChannel._fd = connectfd;
//	_fdConnectMap[connectfd] = con;
//	con->_ref++;
//	//connect->_clientChannel._event = EPOLLIN;
//
//	//connect->_serverChannel._fd = socks5fd;
//
//	//_connectMap[socks5fd] = connect;
//
//}
//
////AUTH --身份认证
////0 表示数据没到，继续等待
////1 表示成功
////-1 失败
//int TransferServer::AuthHandle(int fd)
//{
//	char buf[260];
//	int rlen = recv(fd, buf, 260, MSG_PEEK);
//	TraceDebug("recv:%d", rlen);
//
//	if (rlen < 3)
//		return 0;
//	else if (rlen == -1)
//	{
//		return -1;
//	}
//	else
//	{
//		recv(fd, buf, rlen, 0);
//		if (buf[0] != 0x05)//socks5格式不对
//		{
//			ErrorDebug("not socks5");
//			return -1;
//		}
//		return 1;
//	}
//}
//
////建立连接 接收数据
////-1       失败 
////-2      数据没到
////serverfd  成功
//int TransferServer::EstablishmentHandle(int fd)
//{
//	char buf[256];
//	int rlen = recv(fd, buf, 256, MSG_PEEK);
//	if (rlen < 10)
//	{
//		return -2;
//	}
//	else if (rlen == -1)
//	{
//		return -1;
//	}
//	else
//	{
//		char ip[4];
//		char port[2];
//
//		recv(fd, buf, 4, 0);
//		char addresstype = buf[3];
//		if (addresstype == 0x01)//ipv4
//		{
//			recv(fd, ip, 4, 0);
//			recv(fd, port, 2, 0);
//		}
//		else if (addresstype == 0x03)//domainname
//		{
//			char len = 0;
//			//recv  domainname
//			recv(fd, &len, 1, 0);
//			recv(fd, buf, len, 0); 
//			//recv port
//			recv(fd, port, 2, 0);
//
//			buf[len] = '\0';
//			TraceDebug("domainname: &s", buf);
//			
//			struct hostent* hostptr = gethostbyname(buf);
//			memcpy(ip, hostptr->h_addr, hostptr->h_length);
//		}
//		else if (addresstype == 0x04) //ipv6
//		{
//			ErrorDebug("not support ipv6");
//			return -1;
//		}
//		else
//		{
//			ErrorDebug("invalid address type error");
//			return -1;
//		}
//
//		struct sockaddr_in addr;
//		memset(&addr, 0, sizeof(struct sockaddr_in));
//		addr.sin_family = AF_INET;
//		memcpy(&addr.sin_addr.s_addr, ip, 4);
//		addr.sin_port = *((uint16_t*)port);
//
//		int serverfd = socket(AF_INET, SOCK_STREAM, 0);
//		if (serverfd < 0)
//		{
//			ErrorDebug("server socket error");
//			return - 1;
//		}
//		if (connect(serverfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
//		{
//			ErrorDebug("connect error");
//			close(serverfd);
//			return -1;
//		}
//		return serverfd;
//	}
//}
//
//void TransferServer::RemoveConnect(int fd)
//{
//	OPEvent(fd, EPOLLIN, EPOLL_CTL_DEL);
//	map<int, Connect*>::iterator it = _fdConnectMap.find(fd);
//	if (it != _fdConnectMap.end())
//	{
//		Connect* con = it->second;
//		if (--con->_ref == 0)
//		{
//			delete con;
//			_fdConnectMap.erase(it);
//		}
//	}
//	else
//	{
//		assert(false);
//	}
//}
//
//void TransferServer::Forwarding(Channel* clientChannel, Channel* serverChannel)
//{
//	char buf[4096];
//	int rlen = recv(clientChannel->_fd, buf, 4096, 0);
//	if (rlen < 0)
//	{
//		ErrorDebug("recv: %d", clientChannel->_fd);
//	}
//	else if (rlen == 0)
//	{
//		//client channel发起关闭
//		shutdown(serverChannel->_fd, SHUT_WR);
//		RemoveConnect(clientChannel->_fd);
//	}
//	else
//	{
//		//? 
//		int slen = send(serverChannel->_fd, buf, rlen, 0);
//		TraceDebug("recv:%d  send:%d", rlen, slen);
//	}
//}
//
//void TransferServer::ReadEventHandle(int connectfd)
//{
//	TraceDebug("recv event:%d", connectfd);
//	map<int, Connect*>::iterator it = _fdConnectMap.find(connectfd);
//	if (it != _fdConnectMap.end())
//	{
//		Connect* con = it->second;
//		if (con->_state == AUTH)
//		{
//			char reply[2];  //回复字节
//			reply[0] = 0x05;
//			int ret = AuthHandle(connectfd);//身份认证Auth
//			if (ret == 0)
//			{
//				return;
//			}
//			else if (ret == 1)
//			{
//				reply[1] = 0x00;
//				con->_state = ESTABLISHMENT;
//			}
//			else if (ret == -1)
//			{
//				reply[1] = 0xFF;
//				reply[3] = 0x01;
//			}
//
//			if (send(connectfd, reply, 2, 0) != 2)
//			{
//				ErrorDebug("auth reply");
//			}
//		}
//		else if (con->_state == ESTABLISHMENT)
//		{
//			//回复
//			char reply[10] = { 0 };
//			reply[0] = 0x05;
//			
//			int serverfd = EstablishmentHandle(connectfd);
//			if (serverfd == -1)
//			{
//				ErrorDebug(" EstablishmentHandle error");
//				reply[1] = 0x01;
//				//RemoveConnect(connectfd);
//			}
//			else if (serverfd == -2)
//			{
//				return;
//			}
//			else
//			{
//				reply[1] = 0x00;
//			}
//
//			if (send(connectfd, reply, 10, 0) != 10)
//			{
//				ErrorDebug("Establishment reply error");
//			}
//
//			SetNonblocking(serverfd);
//			OPEvent(serverfd, EPOLLIN, EPOLL_CTL_ADD);
//			con->_serverChannel._fd = serverfd;
//			_fdConnectMap[serverfd] = con;
//			con->_ref++;
//			con->_state = FORWARDING;
//
//		}
//		else if (con->_state == FORWARDING)
//		{
//			Channel* clientChannel = &con->_clientChannel;
//			Channel* serverChannel = &con->_serverChannel;
//
//			if (connectfd == serverChannel->_fd)
//				swap(clientChannel, serverChannel);
//
//			//client  ->  server
//			Forwarding(clientChannel, serverChannel);
//		}
//		else
//		{
//			assert(false);
//		}
//		//Channel* clientChannel = &(con->_clientChannel);
//		//Channel* serverChannel = &(con->_serverChannel);
//
//		//if (fd == serverChannel->_fd)
//		//{
//		//	swap(clientChannel, serverChannel);
//		//}
//		//	
//		//Forwording(clientChannel, serverChannel);
//	}
//	else
//	{
//		assert(false);
//	}
//}
//void TransferServer::WriteEventHandle(int connectfd)
//{
//	TraceDebug("WriteEventHandle");
//}
//
//int main()
//{
//	TransferServer server("192.168.153.128", 8001, 8000);
//	server.Start();
//}