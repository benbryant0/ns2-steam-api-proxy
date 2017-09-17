#include "stdafx.h"

#include "steam_api.h"
#include "SteamTypes.h"
#include "PipeServer.h"
#include "PipePackets.h"

#define STEAM_API extern "C" __declspec( dllexport )
#define LISTEN_PIPE "\\\\.\\pipe\\NS2Assistant"

std::unique_ptr<PipeServer> Pipe;
HMODULE SteamApiDll;
CCallbackBase* ServerChangeRequestedCallback;

typedef void (*SteamAPI_RegisterCallbackFn)(CCallbackBase*, int);
SteamAPI_RegisterCallbackFn RegisterCallback;

typedef void(*SteamAPI_ActivateGameOverlayToWebPageFn)(void*, const char *);
SteamAPI_ActivateGameOverlayToWebPageFn ActivateGameOverlayToWebPage;

typedef void*(*SteamFriendsFn)();
SteamFriendsFn GetSteamFriends;

bool LoadSteamApi()
{
	if (SteamApiDll != nullptr)
		return true;

	if ((SteamApiDll = LoadLibraryA("steam_api_original.dll")) == nullptr)
	{
		return false;
	}

	const auto tryLoad = []() {
		RegisterCallback = reinterpret_cast<SteamAPI_RegisterCallbackFn>(GetProcAddress(SteamApiDll, "SteamAPI_RegisterCallback"));
		if (RegisterCallback == nullptr)
			return false;

		ActivateGameOverlayToWebPage = reinterpret_cast<SteamAPI_ActivateGameOverlayToWebPageFn>(GetProcAddress(SteamApiDll, "SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage"));
		if (ActivateGameOverlayToWebPage == nullptr)
			return false;

		GetSteamFriends = reinterpret_cast<SteamFriendsFn>(GetProcAddress(SteamApiDll, "SteamFriends"));
		if (GetSteamFriends == nullptr)
			return false;
	};

	if (!tryLoad())
	{
		if (FreeLibrary(SteamApiDll) != 0)
			SteamApiDll = nullptr;

		return false;
	}

	return true;
}

void PipeDataReceived(std::unique_ptr<PipePacket>& packet)
{
	if (!LoadSteamApi()) return;

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
		ActivateGameOverlayToWebPage(GetSteamFriends(), openOverlayPacket->Url);
	}
}

STEAM_API void SteamAPI_RegisterCallback(CCallbackBase* pCallback, int iCallback)
{
	if (!LoadSteamApi()) return;

	RegisterCallback(pCallback, iCallback);

	if (iCallback == 332)
		ServerChangeRequestedCallback = pCallback;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		Pipe = std::make_unique<PipeServer>(LISTEN_PIPE, PipeDataReceived);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		if (Pipe->Running())
			Pipe->Stop();
	}

	return 1;
}
