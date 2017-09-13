#include "stdafx.h"

#include "PipePackets.h"

std::unique_ptr<PipePacket> PipePacket::Deserialize(char * data, int len)
{
	if (len < 5) return nullptr;

	const auto type = static_cast<PipePacketType>(data[0]);
	if (type >= PipePacketType::EnumLimit) return nullptr;

	const int dataLen = *reinterpret_cast<int*>(data + 1);

	std::unique_ptr<PipePacket> packet;
	switch (type)
	{
		case PipePacketType::ServerChangePacket:
			packet = std::make_unique<ServerChangePacket>();
			break;
		case PipePacketType::OpenOverlayUrlPacket:
			packet = std::make_unique<OpenOverlayUrlPacket>();
			break;
		default:
			return nullptr;
	}

	if (dataLen != packet->Size())
		return nullptr;

	packet->Deserialize(data + 5);

	return packet;
}
