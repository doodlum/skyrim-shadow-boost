#include "ShadowBoost.h"

void ShadowBoost::LoadJSON()
{
	std::ifstream i(L"Data\\SKSE\\Plugins\\ShadowBoost.json");
	i >> JSONSettings;
	Enabled = JSONSettings["Enabled"];
	TargetMS = JSONSettings["Target CPU Frametime (ms)"];
	RateOfChange = JSONSettings["Distance Change Speed"];
	MaxDistance = JSONSettings["Maximum Distance"];
	MinDistance = JSONSettings["Minimum Distance"];
}

void ShadowBoost::SaveJSON()
{
	std::ofstream o(L"Data\\SKSE\\Plugins\\ShadowBoost.json");
	JSONSettings["Enabled"] = Enabled;
	JSONSettings["Target CPU Frametime (ms)"] = TargetMS;
	JSONSettings["Distance Change Speed"] = RateOfChange;
	JSONSettings["Maximum Distance"] = MaxDistance;
	JSONSettings["Minimum Distance"] = MinDistance;
	o << JSONSettings.dump(1);
}

void ShadowBoost::Start()
{
	init = true;
}

void ShadowBoost::UpdateShadows(float a_avgTiming)
{
	if (Enabled) {
		float timeRatio = a_avgTiming / (TargetMS / 1000);
		float scaleRatio = (RateOfChange / (MaxDistance - MinDistance)) * (1.0f - timeRatio) + 1.0f;
		gShadowDistance = std::clamp(gShadowDistance * scaleRatio, MinDistance, MaxDistance);
	} else {
		gShadowDistance = MaxDistance;
	}
	UpdateShadowDistance();
}

void ShadowBoost::UpdateSettings()
{
	long long finish = PerformanceCounter();
	double    elapsedMilliseconds = (double)((finish - frameEnd)) / PerformanceFrequency();
	float     frameTime = (float)elapsedMilliseconds;
	float     avgTiming;
	if (HaveUpdatedAveragedTiming(frameTime, avgTiming))
		UpdateShadows(frameTime);
	lastCPUFrameTime = frameTime;
}

void ShadowBoost::Update()
{
	if (init)
		UpdateSettings();
	frameEnd = PerformanceCounter();
}

bool ShadowBoost::HaveUpdatedAveragedTiming(float a_frametime, float& a_avgTiming)
{
	frameCounter++;
	bool badframe = frameCounter >= CPUTimingKeepNumFrames;
	if (badframe) {
		for (int i = CPUTimingKeepNumFrames - 1; i > 0; i--) {
			timingPerFrame[i] = timingPerFrame[i - 1];
		}
	} else {
		a_avgTiming = 0.0f;
		for (int i = CPUTimingKeepNumFrames - 1; i > 0; i--) {
			timingPerFrame[i] = timingPerFrame[i - 1];
			a_avgTiming += timingPerFrame[i];
		}
		a_avgTiming += a_frametime;
		a_avgTiming /= CPUTimingKeepNumFrames;
	}
	timingPerFrame[0] = a_frametime;
	return badframe;
}

void ShadowBoost::UISetMaxShadowDistance(const void* value, [[maybe_unused]] void* clientData)
{
	GetSingleton()->MaxDistance = max(GetSingleton()->MinDistance, *static_cast<const float*>(value));
}

void ShadowBoost::UIGetMaxShadowDistance(void* value, [[maybe_unused]] void* clientData)
{
	*static_cast<float*>(value) = GetSingleton()->MaxDistance;
}

void ShadowBoost::UISetMinShadowDistance(const void* value, [[maybe_unused]] void* clientData)
{
	GetSingleton()->MinDistance = min(*static_cast<const float*>(value), GetSingleton()->MaxDistance);
}

void ShadowBoost::UIGetMinShadowDistance(void* value, [[maybe_unused]] void* clientData)
{
	*static_cast<float*>(value) = GetSingleton()->MinDistance;
}

void ShadowBoost::UpdateUI()
{
	auto bar = g_ENB->TwGetBarByEnum(ENB_API::ENBWindowType::GeneralWindow);
	g_ENB->TwAddVarRW(bar, "Enabled", ETwType::TW_TYPE_BOOLCPP, &Enabled, "group=MOD:ShadowBoost");
	g_ENB->TwAddVarRW(bar, "Target CPU Frametime (ms)", ETwType::TW_TYPE_FLOAT, &TargetMS, "group=MOD:ShadowBoost");
	g_ENB->TwAddVarRW(bar, "Distance Change Speed", ETwType::TW_TYPE_FLOAT, &RateOfChange, "group=MOD:ShadowBoost  min=0.00 max=1000.0");
	g_ENB->TwAddVarCB(bar, "Maximum Distance", ETwType::TW_TYPE_FLOAT, UISetMaxShadowDistance, UIGetMaxShadowDistance, this, "group=MOD:ShadowBoost min=1.00 max=1000000.0");
	g_ENB->TwAddVarCB(bar, "Minimum Distance", ETwType::TW_TYPE_FLOAT, UISetMinShadowDistance, UIGetMinShadowDistance, this, "group=MOD:ShadowBoost min=1.00 max=1000000.0");
	g_ENB->TwAddVarRW(bar, "Current Distance", ETwType::TW_TYPE_FLOAT, &gShadowDistance, "group=MOD:ShadowBoost readonly=true");
	g_ENB->TwAddVarRW(bar, "Last CPU Frametime", ETwType::TW_TYPE_FLOAT, &lastCPUFrameTime, "group=MOD:ShadowBoost readonly=true");
}
