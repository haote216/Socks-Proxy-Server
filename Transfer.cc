#include"Transfer.h"

void TransferServer::ConnectEventHandle(int fd)
{
	//Á¬½Ósocks
	TraceLog("ConnectEventHandle");

}
void TransferServer::ReadEventHandle(int fd)
{
	TraceLog("ReadEventHandle");
}
void TransferServer::WriteEventHandle(int fd)
{
	TraceLog("WriteEventHandle");
}

int main()
{
	TransferServer server("123.207.58.25", 8001, 8000);
	server.Start();
}
