#include "stdafx.h"

#include "PipeServer.h"

PipeServer::PipeServer(std::string pipePath, PacketReceivedHandler_t PipeReceivedHandler)
	: PacketReceivedHandler(PipeReceivedHandler)
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

	const int recvBufLen = 512;
	char recvbuf[recvBufLen];

	while (Server->_Running)
	{
		const bool fConnected = ConnectNamedPipe(Server->PipeHandle, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fConnected)
		{
			unsigned long cbBytesRead;
			const bool fSuccess = ReadFile(
				Server->PipeHandle,
				recvbuf,
				recvBufLen,
				&cbBytesRead,
				nullptr);

			if (fSuccess)
			{
				std::unique_ptr<PipePacket> packet = PipePacket::Deserialize(recvbuf, cbBytesRead);
				if(packet != nullptr)
					Server->PacketReceivedHandler(packet);
			}

			DisconnectNamedPipe(Server->PipeHandle);
		}
	}
	return 0;
}
