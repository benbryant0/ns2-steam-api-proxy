#pragma once

enum class PipePacketType : unsigned char
{
	ServerChangePacket = 0,
	OpenOverlayUrlPacket = 1,
	EnumLimit
};

struct PipePacket
{
	virtual ~PipePacket() = default;
	virtual PipePacketType Type() const = 0;
	virtual size_t Size() const = 0;
	static std::unique_ptr<PipePacket> Deserialize(char * data, int len);
private:
	virtual void Deserialize(const char* data) = 0;
};

struct ServerChangePacket : PipePacket
{
	char Server[64];	// server address ("127.0.0.1:27015", "tf2.valvesoftware.com")
	char Password[64];	// server password, if any

	PipePacketType Type() const override
	{
		return PipePacketType::ServerChangePacket;
	}

	size_t Size() const override
	{
		return sizeof Server + sizeof Password;
	}

private:
	void Deserialize(const char* data) override
	{
		strncpy_s(Server, data, sizeof Server - 1);
		strncpy_s(Password, data + sizeof Server, sizeof Password - 1);
	}
};

struct OpenOverlayUrlPacket : PipePacket
{
	char Url[256];

	PipePacketType Type() const override
	{
		return PipePacketType::OpenOverlayUrlPacket;
	}

	size_t Size() const override
	{
		return sizeof Url;
	}

private:
	void Deserialize(const char* data) override
	{
		strncpy_s(Url, data, sizeof Url - 1);
	}
};
