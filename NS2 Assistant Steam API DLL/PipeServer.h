#pragma once

#include "FriendsCommon.h"

class PipeServer
{
private:
	HANDLE PipeHandle;
	static DWORD WINAPI MainLoop(LPVOID lParam);
	bool _Running = false;
	void(&DataReceivedHandler)(GameServerChangeRequested_t&);
public:
	PipeServer(std::string pipePath, void(&DataReceivedHandler)(GameServerChangeRequested_t&));
	~PipeServer();
	bool Running() const { return _Running; }
	void Stop();
};
