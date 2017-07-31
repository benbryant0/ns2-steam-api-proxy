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
