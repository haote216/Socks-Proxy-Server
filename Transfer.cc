#include"Transfer.h"

void TransferServer::ConnectEventHandle(int fd)
{
	//
	//Á¬½Ósocks
	TraceDebug("ConnectEventHandle");

}
void TransferServer::ReadEventHandle(int fd)
{
	TraceDebug("ReadEventHandle");
}
void TransferServer::WriteEventHandle(int fd)
{
	TraceDebug("WriteEventHandle");
}

int main()
{
	TransferServer server("192.168.153.128", 8001, 8000);
	server.Start();
}