#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "API/ENBSeriesAPI.h"
#include <shared_mutex>

//constexpr auto CPUTimingKeepNumFrames = 3;

/*static long&  gLastUpdatePerformanceCounterTick = (*(long*)RELOCATION_ID(524950, 411438).address());
static float& gQueryPerformanceTickIntervalInMilliseconds = (*(float*)RELOCATION_ID(511886, 388446).address())*/
;

static float& fShadowDistance = (*(float*)RELOCATION_ID(513793, 391845).address());
static float& fLODFadeOutMultObjects = (*(float*)RELOCATION_ID(500935, 358960).address());
static float& fLODFadeOutMultItems = (*(float*)RELOCATION_ID(500933, 358957).address());
static float& fLODFadeOutMultActors = (*(float*)RELOCATION_ID(500931, 358954).address());

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
	json                    JSONSettings;

	// GameSettings

	float& GetGameSettingFloat(std::string a_name, std::string a_section);

	// Settings

	bool ShadowsEnabled;
	bool LODEnabled;

	float TargetFPS;
	float RateOfChange;

	float fShadowDistanceMax;
	float fShadowDistanceMin;

	float fLODFadeOutMultObjectsMax;
	float fLODFadeOutMultObjectsMin;

	float fLODFadeOutMultItemsMax;
	float fLODFadeOutMultItemsMin;

	float fLODFadeOutMultActorsMax;
	float fLODFadeOutMultActorsMin;

	// Variables

	bool init = false;
	//int   frameCounter = 0;
	float lastCPUFrameTime;
	//float timingPerFrame[CPUTimingKeepNumFrames];

	void LoadJSON();
	void SaveJSON();
	void Start();

	void UpdateShadows(float a_avgTiming);
	void UpdateSettings();
	void Update();

	// Performance Queries

	//bool HaveUpdatedAveragedTiming(float a_frametime, float& a_avgTiming);

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

	long long frameEnd = PerformanceCounter();

	static void UpdateShadowDistanceAndInteriorFlag(bool a_isInterior)
	{
		using func_t = decltype(&UpdateShadowDistanceAndInteriorFlag);
		REL::Relocation<func_t> func{ REL::RelocationID(98978, 105631) };
		func(a_isInterior);
	}

	static void UpdateShadowDistance()
	{
		static const bool& gIsInterior = (*(bool*)RELOCATION_ID(513222, 390962).address());
		UpdateShadowDistanceAndInteriorFlag(gIsInterior);
	}

	// ENB UI

#define UI_SETTER_GETTER_DISTANCE(maxparam, minparam)                                                \
	static void UISet##maxparam(const void* value, [[maybe_unused]] void* clientData)                \
	{                                                                                                \
		GetSingleton()->maxparam = max(GetSingleton()->minparam, *static_cast<const float*>(value)); \
	};                                                                                               \
	static void UIGet##maxparam(void* value, [[maybe_unused]] void* clientData)                      \
	{                                                                                                \
		*static_cast<float*>(value) = GetSingleton()->maxparam;                                      \
	}                                                                                                \
	static void UISet##minparam(const void* value, [[maybe_unused]] void* clientData)                \
	{                                                                                                \
		GetSingleton()->minparam = min(GetSingleton()->maxparam, *static_cast<const float*>(value)); \
	}                                                                                                \
	static void UIGet##minparam(void* value, [[maybe_unused]] void* clientData)                      \
	{                                                                                                \
		*static_cast<float*>(value) = GetSingleton()->minparam;                                      \
	}

	UI_SETTER_GETTER_DISTANCE(fShadowDistanceMax, fShadowDistanceMin)

	UI_SETTER_GETTER_DISTANCE(fLODFadeOutMultObjectsMax, fLODFadeOutMultObjectsMin)
	UI_SETTER_GETTER_DISTANCE(fLODFadeOutMultItemsMax, fLODFadeOutMultItemsMin)
	UI_SETTER_GETTER_DISTANCE(fLODFadeOutMultActorsMax, fLODFadeOutMultActorsMin)

#undef UI_SETTER_GETTER_DISTANCE

	void UpdateUI();

protected:
	struct Hooks
	{
		struct TimeManager_SleepCheck
		{
			static INT64 thunk(int a1)
			{
				INT64 ret = func(a1);
				GetSingleton()->Update();
				GetSingleton()->frameEnd = PerformanceCounter();
				return ret;
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct TimeManager_Sleep
		{
			static LARGE_INTEGER thunk()
			{
				GetSingleton()->frameEnd = PerformanceCounter();
				return func();
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_thunk_call<TimeManager_SleepCheck>(REL::RelocationID(75461, 77246).address() + REL::Relocate(0x9, 0x9));
			stl::write_thunk_call<TimeManager_Sleep>(REL::RelocationID(75461, 77246).address() + REL::Relocate(0x44, 0xAF));
		}
	};

private:
	ShadowBoost()
	{
		LoadJSON();
	};

	ShadowBoost(const ShadowBoost&) = delete;
	ShadowBoost(ShadowBoost&&) = delete;

	~ShadowBoost() = default;

	ShadowBoost& operator=(const ShadowBoost&) = delete;
	ShadowBoost& operator=(ShadowBoost&&) = delete;
};
