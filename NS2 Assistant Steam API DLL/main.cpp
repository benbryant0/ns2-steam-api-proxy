#include "stdafx.h"

#include "steam_api.h"
#include "SteamTypes.h"
#include "PipeServer.h"
#include "PipePackets.h"

#define STEAM_API extern "C" __declspec( dllexport )
#define LISTEN_PIPE "\\\\.\\pipe\\NS2Assistant"

std::unique_ptr<PipeServer> Pipe;
HMODULE SteamApiDll;
typedef void (*SteamAPI_RegisterCallbackFn)(CCallbackBase*, int);
SteamAPI_RegisterCallbackFn RegisterCallback_Original;
CCallbackBase* ServerChangeRequestedCallback;

typedef void(*SteamAPI_ActivateGameOverlayToWebPageFn)(void*, const char *);
SteamAPI_ActivateGameOverlayToWebPageFn ActivateGameOverlayToWebPage_Original;

typedef void*(*SteamFriendsFn)();
SteamFriendsFn GetSteamFriends;

void PipeDataReceived(std::unique_ptr<PipePacket>& packet)
{
	const auto type = packet->Type();
	if(type == PipePacketType::ServerChangePacket && ServerChangeRequestedCallback != nullptr)
	{
		const auto serverChangePacket = dynamic_cast<ServerChangePacket*>(packet.get());
		GameServerChangeRequested_t changeRequest;
		memcpy(changeRequest.m_rgchServer, serverChangePacket->Server, sizeof changeRequest.m_rgchServer);
		memcpy(changeRequest.m_rgchPassword, serverChangePacket->Password, sizeof changeRequest.m_rgchPassword);
		ServerChangeRequestedCallback->Run(&changeRequest);
	}
	else if(type == PipePacketType::OpenOverlayUrlPacket)
	{
		const auto openOverlayPacket = dynamic_cast<OpenOverlayUrlPacket*>(packet.get());
		ActivateGameOverlayToWebPage_Original(GetSteamFriends(), openOverlayPacket->Url);
	}
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		SteamApiDll = LoadLibraryA("steam_api_original.dll");
		if (SteamApiDll == nullptr)
			return 1;

		RegisterCallback_Original = reinterpret_cast<SteamAPI_RegisterCallbackFn>(GetProcAddress(SteamApiDll, "SteamAPI_RegisterCallback"));
		if (RegisterCallback_Original == nullptr)
			return 1;

		ActivateGameOverlayToWebPage_Original = reinterpret_cast<SteamAPI_ActivateGameOverlayToWebPageFn>(GetProcAddress(SteamApiDll, "SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage"));
		if (ActivateGameOverlayToWebPage_Original == nullptr)
			return 1;

		GetSteamFriends = reinterpret_cast<SteamFriendsFn>(GetProcAddress(SteamApiDll, "SteamFriends"));
		if (GetSteamFriends == nullptr)
			return 1;

		Pipe = std::make_unique<PipeServer>(LISTEN_PIPE, PipeDataReceived);
	}
	else if(dwReason == DLL_PROCESS_DETACH)
	{
		if (Pipe->Running())
			Pipe->Stop();

		FreeLibrary(SteamApiDll);
	}

	return 1;
}

STEAM_API void SteamAPI_RegisterCallback(CCallbackBase* pCallback, int iCallback)
{
	RegisterCallback_Original(pCallback, iCallback);

	if (iCallback == 332)
		ServerChangeRequestedCallback = pCallback;
}