#include "stdafx.h"

#include "steam_api.h"
#include "CCallback.h"
#include "PipeServer.h"

#define STEAM_API extern "C" __declspec( dllexport )
#define LISTEN_PIPE "\\\\.\\pipe\\NS2Assistant_JoinServer"

std::shared_ptr<PipeServer> Pipe;
HMODULE SteamApiDll;
typedef void (*SteamAPI_RegisterCallbackFn)(CCallbackBase*, int);
SteamAPI_RegisterCallbackFn NativeRegisterCallback;
CCallbackBase* ServerChangeRequestedCallback;

void SocketDataReceived(GameServerChangeRequested_t& data)
{
	if(ServerChangeRequestedCallback != nullptr)
		ServerChangeRequestedCallback->Run(&data);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		SteamApiDll = LoadLibraryA("steam_api_original.dll");
		if (SteamApiDll == nullptr)
			return 1;
		NativeRegisterCallback = reinterpret_cast<SteamAPI_RegisterCallbackFn>(GetProcAddress(SteamApiDll, "SteamAPI_RegisterCallback"));
		if (NativeRegisterCallback == nullptr)
			return 1;

		Pipe = std::make_shared<PipeServer>(LISTEN_PIPE, SocketDataReceived);
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
	NativeRegisterCallback(pCallback, iCallback);

	if (iCallback == 332)
		ServerChangeRequestedCallback = pCallback;
}