#pragma once

class CCallbackBase
{
public:
	CCallbackBase() { m_nCallbackFlags = 0; m_iCallback = 0; }
	// don't add a virtual destructor because we export this binary interface across dll's
	virtual void Run(void *pvParam) = 0;
	virtual void Run(void *pvParam, bool bIOFailure, unsigned long hSteamAPICall) = 0;
	int GetICallback() { return m_iCallback; }
	virtual int GetCallbackSizeBytes();
	enum { k_ECallbackFlagsRegistered = 0x01, k_ECallbackFlagsGameServer = 0x02 };
	unsigned long m_nCallbackFlags;
	int m_iCallback;
	friend class CCallbackMgr;
};

struct GameServerChangeRequested_t
{
	enum { k_iCallback = 332 };
	char m_rgchServer[64];		// server address ("127.0.0.1:27015", "tf2.valvesoftware.com")
	char m_rgchPassword[64];	// server password, if any
};
