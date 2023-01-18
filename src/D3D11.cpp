#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>

#include <Detours.h>

#include "ShadowBoost.h"

decltype(&IDXGISwapChain::Present)         ptrPresent;

HRESULT WINAPI hk_IDXGISwapChain_Present(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
{
	ShadowBoost::GetSingleton()->Update();
	return (This->*ptrPresent)(SyncInterval, Flags);
}

struct Hooks
{
	struct BSGraphics_Renderer_Init_InitD3D
	{
		static void thunk()
		{
			logger::info("Calling original Init3D");
			func();
			logger::info("Accessing render device information");
			auto manager = RE::BSRenderManager::GetSingleton();
			*(uintptr_t*)&ptrPresent = Detours::X64::DetourClassVTable(*(uintptr_t*)manager->GetRuntimeData().swapChain, &hk_IDXGISwapChain_Present, 8);

			logger::info("Detouring virtual function tables");
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	static void Install()
	{
		stl::write_thunk_call<BSGraphics_Renderer_Init_InitD3D>(REL::RelocationID(75595, 77226).address() + REL::Relocate(0x50, 0x2BC));
		logger::info("Installed render startup hooks");
	}
};

void PatchD3D11()
{
	Hooks::Install();
}
