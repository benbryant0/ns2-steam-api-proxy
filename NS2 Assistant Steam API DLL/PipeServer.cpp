#include "stdafx.h"

#include "PipeServer.h"

PipeServer::PipeServer(std::string pipePath, void(&DataReceivedHandler)(GameServerChangeRequested_t&))
	: DataReceivedHandler(DataReceivedHandler)
{
	this->PipeHandle = CreateNamedPipeA(pipePath.c_str(), PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 0, 128, 0, NULL);

	if (this->PipeHandle == INVALID_HANDLE_VALUE)
		return;

	_Running = true;

	CreateThread(NULL, NULL, &PipeServer::MainLoop, this, NULL, NULL);
}

PipeServer::~PipeServer()
{
	if (this->_Running)
		this->Stop();
}

void PipeServer::Stop()
{
	if (!this->_Running)
		return;

	this->_Running = false;

	if (this->PipeHandle != NULL && this->PipeHandle != INVALID_HANDLE_VALUE)
	{
		DisconnectNamedPipe(this->PipeHandle);
		CloseHandle(this->PipeHandle);
	}
}

DWORD WINAPI PipeServer::MainLoop(LPVOID lParam)
{
	PipeServer* Server = (PipeServer*)lParam;

	const int recvbuflen = sizeof(ServerChangePacket);
	char recvbuf[recvbuflen];

	ServerChangePacket packet;

	while (Server->_Running)
	{
		bool fConnected = ConnectNamedPipe(Server->PipeHandle, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fConnected)
		{
			unsigned long cbBytesRead;
			bool fSuccess = ReadFile(
				Server->PipeHandle,
				recvbuf,
				recvbuflen,
				&cbBytesRead,
				NULL);

			if (fSuccess && cbBytesRead == 128 && recvbuf[63] == 0x00 && recvbuf[127] == 0x00)
			{
				packet.deserialize(recvbuf);
				Server->DataReceivedHandler((GameServerChangeRequested_t)packet);
			}

			DisconnectNamedPipe(Server->PipeHandle);
		}
	}
	return 0;
}
