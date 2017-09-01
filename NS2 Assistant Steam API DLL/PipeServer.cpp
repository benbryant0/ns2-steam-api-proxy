#include "stdafx.h"

#include "PipeServer.h"

PipeServer::PipeServer(std::string pipePath, void(&DataReceivedHandler)(GameServerChangeRequested_t&))
	: DataReceivedHandler(DataReceivedHandler)
{
	this->PipeHandle = CreateNamedPipeA(pipePath.c_str(), PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 0, 128, 0, nullptr);

	if (this->PipeHandle == INVALID_HANDLE_VALUE)
		return;

	_Running = true;

	CreateThread(nullptr, NULL, &PipeServer::MainLoop, this, NULL, nullptr);
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

	if (this->PipeHandle != nullptr && this->PipeHandle != INVALID_HANDLE_VALUE)
	{
		DisconnectNamedPipe(this->PipeHandle);
		CloseHandle(this->PipeHandle);
	}
}

DWORD WINAPI PipeServer::MainLoop(LPVOID lParam)
{
	PipeServer* Server = static_cast<PipeServer*>(lParam);

	const int recvbuflen = sizeof(GameServerChangeRequested_t);
	char recvbuf[recvbuflen];

	GameServerChangeRequested_t packet;

	while (Server->_Running)
	{
		const bool fConnected = ConnectNamedPipe(Server->PipeHandle, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fConnected)
		{
			unsigned long cbBytesRead;
			const bool fSuccess = ReadFile(
				Server->PipeHandle,
				recvbuf,
				recvbuflen,
				&cbBytesRead,
				nullptr);

			if (fSuccess && cbBytesRead == 128 && recvbuf[63] == 0x00 && recvbuf[127] == 0x00)
			{
				packet.deserialize(recvbuf);
				Server->DataReceivedHandler(packet);
			}

			DisconnectNamedPipe(Server->PipeHandle);
		}
	}
	return 0;
}
