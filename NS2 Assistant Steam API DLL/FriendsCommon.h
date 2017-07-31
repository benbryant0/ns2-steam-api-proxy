#pragma once

struct GameServerChangeRequested_t
{
enum { k_iCallback = 332 };
char m_rgchServer[64];		// server address ("127.0.0.1:27015", "tf2.valvesoftware.com")
char m_rgchPassword[64];	// server password, if any
};
