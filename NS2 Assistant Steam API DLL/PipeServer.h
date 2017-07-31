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

struct ServerChangePacket : GameServerChangeRequested_t
{
	void serialize(char * dat) {
		memcpy(dat, this, sizeof(ServerChangePacket));
	}

	void deserialize(const char * dat) {
		memcpy(this, dat, sizeof(ServerChangePacket));
	}
};
