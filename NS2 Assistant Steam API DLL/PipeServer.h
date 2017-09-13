#pragma once

#include "PipePackets.h"

typedef void(&PacketReceivedHandler_t)(std::unique_ptr<PipePacket>&);

class PipeServer
{
	HANDLE PipeHandle;
	static DWORD WINAPI MainLoop(LPVOID lParam);
	bool _Running = false;
	PacketReceivedHandler_t PacketReceivedHandler;
public:
	PipeServer(std::string pipePath, PacketReceivedHandler_t);
	~PipeServer();
	bool Running() const { return _Running; }
	void Stop();
};
