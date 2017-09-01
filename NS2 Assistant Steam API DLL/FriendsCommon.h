#pragma once

struct GameServerChangeRequested_t
{
	enum { k_iCallback = 332 };
	char m_rgchServer[64];		// server address ("127.0.0.1:27015", "tf2.valvesoftware.com")
	char m_rgchPassword[64];	// server password, if any

	void serialize(char * dat) const
	{
		memcpy(dat, this, sizeof(GameServerChangeRequested_t));
	}

	void deserialize(const char * dat)
	{
		memcpy(this, dat, sizeof(GameServerChangeRequested_t));
	}
};
