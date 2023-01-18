#pragma once

#include "ENB/ENBSeriesAPI.h"
#include <shared_mutex>

class ShadowBoost
{
public:
	static ShadowBoost* GetSingleton()
	{
		static ShadowBoost handler;
		return &handler;
	}

	static void InstallHooks()
	{
		Hooks::Install();
	}

	ENB_API::ENBSDKALT1001* g_ENB = nullptr;
	std::shared_mutex       fileLock;

	// GameSettings

	float& GetGameSettingFloat(std::string a_name, std::string a_section);

	// Settings

	float fTargetFPS;
	float fRateOfChange;

	// Shadows

	bool bShadowsEnabled;

	float fShadowDistanceMax;
	float fShadowDistanceMin;

	// LOD

	bool bLODEnabled;

	float fLODFadeOutMultObjectsMax;
	float fLODFadeOutMultObjectsMin;

	float fLODFadeOutMultItemsMax;
	float fLODFadeOutMultItemsMin;

	float fLODFadeOutMultActorsMax;
	float fLODFadeOutMultActorsMin;

	// Terrain

	bool bTerrainEnabled;

	float fBlockLevel0DistanceMax;
	float fBlockLevel0DistanceMin;

	float fBlockLevel1DistanceMax;
	float fBlockLevel1DistanceMin;

	float fTreeLoadDistanceMax;
	float fTreeLoadDistanceMin;

	// Variables

	bool  init = false;
	float lastCPUFrameTime;

	void LoadINI();
	void SaveINI();
	void Start();

	void UpdateShadows(float a_avgTiming);
	void UpdateSettings();
	void Update();

	// Performance Queries

	static inline long long PerformanceCounter() noexcept
	{
		LARGE_INTEGER li;
		::QueryPerformanceCounter(&li);
		return li.QuadPart;
	}

	static inline long long PerformanceFrequency() noexcept
	{
		LARGE_INTEGER li;
		::QueryPerformanceFrequency(&li);
		return li.QuadPart;
	}

	long long frameStart = PerformanceCounter();

	static void UpdateShadowDistanceAndInteriorFlag(bool a_isInterior)
	{
		using func_t = decltype(&UpdateShadowDistanceAndInteriorFlag);
		REL::Relocation<func_t> func{ RELOCATION_ID(98978, 105631) };
		func(a_isInterior);
	}

	static void UpdateShadowDistance()
	{
		static const bool& gIsInterior = (*(bool*)RELOCATION_ID(513222, 390962).address());
		UpdateShadowDistanceAndInteriorFlag(gIsInterior);
	}

	// ENB UI

#define UI_SETTER_GETTER_DISTANCE(param)                                                \
	static void UISet##param##Max(const void* value, [[maybe_unused]] void* clientData)                \
	{                                                                                                \
		GetSingleton()->param##Max = max(GetSingleton()->param##Min, *static_cast<const float*>(value)); \
	};                                                                                               \
	static void UIGet##param##Max(void* value, [[maybe_unused]] void* clientData)                      \
	{                                                                                                \
		*static_cast<float*>(value) = GetSingleton()->param##Max;                                      \
	}                                                                                                \
	static void UISet##param##Min(const void* value, [[maybe_unused]] void* clientData)                \
	{                                                                                                \
		GetSingleton()->param##Min = min(GetSingleton()->param##Max, *static_cast<const float*>(value)); \
	}                                                                                                \
	static void UIGet##param##Min(void* value, [[maybe_unused]] void* clientData)                      \
	{                                                                                                \
		*static_cast<float*>(value) = GetSingleton()->param##Min;                                      \
	}

	UI_SETTER_GETTER_DISTANCE(fShadowDistance)

	UI_SETTER_GETTER_DISTANCE(fLODFadeOutMultObjects)
	UI_SETTER_GETTER_DISTANCE(fLODFadeOutMultItems)
	UI_SETTER_GETTER_DISTANCE(fLODFadeOutMultActors)

	UI_SETTER_GETTER_DISTANCE(fBlockLevel0Distance)
	UI_SETTER_GETTER_DISTANCE(fBlockLevel1Distance)
	UI_SETTER_GETTER_DISTANCE(fTreeLoadDistance)

#undef UI_SETTER_GETTER_DISTANCE

	void RefreshUI();

protected:
	struct Hooks
	{
		struct Main_Update_Start
		{
			static void thunk(INT64 a_unk)
			{
				GetSingleton()->frameStart = PerformanceCounter();
				func(a_unk);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct Main_Update_Render
		{
			static void thunk(RE::Main* a_main)
			{
				GetSingleton()->Update();
				func(a_main);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_thunk_call<Main_Update_Start>(REL::RelocationID(35565, 36564).address() + REL::Relocate(0x1E, 0x3E, 0x33));
		//	stl::write_thunk_call<Main_Update_Render>(REL::RelocationID(35565, 36564).address() + REL::Relocate(0x5D2, 0xA92, 0x678));
		}
	};

private:
	ShadowBoost()
	{
		LoadINI();
	};

	ShadowBoost(const ShadowBoost&) = delete;
	ShadowBoost(ShadowBoost&&) = delete;

	~ShadowBoost() = default;

	ShadowBoost& operator=(const ShadowBoost&) = delete;
	ShadowBoost& operator=(ShadowBoost&&) = delete;
};
