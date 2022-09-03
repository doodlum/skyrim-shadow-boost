#include "ShadowBoost.h"

float& ShadowBoost::GetGameSettingFloat(std::string a_name, std::string a_section)
{
	auto ini = RE::INISettingCollection::GetSingleton();
	return ini->GetSetting(std::format("{}:{}", a_name, a_section))->data.f;
}

void ShadowBoost::LoadJSON()
{
	std::ifstream i(L"Data\\SKSE\\Plugins\\ShadowBoost.json");
	i >> JSONSettings;
	ShadowsEnabled = JSONSettings["Shadows Enabled"];
	LODEnabled = JSONSettings["LOD Enabled"];

	TargetFPS = JSONSettings["Target FPS"];
	RateOfChange = JSONSettings["Distance Change Speed"];

	fShadowDistanceMax = JSONSettings["fShadowDistanceMax"];
	fShadowDistanceMin = JSONSettings["fShadowDistanceMin"];

	fLODFadeOutMultObjectsMax = JSONSettings["fLODFadeOutMultObjectsMax"];
	fLODFadeOutMultObjectsMin = JSONSettings["fLODFadeOutMultObjectsMin"];

	fLODFadeOutMultItemsMax = JSONSettings["fLODFadeOutMultItemsMax"];
	fLODFadeOutMultItemsMin = JSONSettings["fLODFadeOutMultItemsMin"];

	fLODFadeOutMultActorsMax = JSONSettings["fLODFadeOutMultActorsMax"];
	fLODFadeOutMultActorsMin = JSONSettings["fLODFadeOutMultActorsMin"];
}

void ShadowBoost::SaveJSON()
{
	std::ofstream o(L"Data\\SKSE\\Plugins\\ShadowBoost.json");
	JSONSettings["Shadows Enabled"] = ShadowsEnabled;
	JSONSettings["LOD Enabled"] = LODEnabled;

	JSONSettings["Target FPS"] = TargetFPS;
	JSONSettings["Distance Change Speed"] = RateOfChange;

	JSONSettings["fShadowDistanceMax"] = fShadowDistanceMax;
	JSONSettings["fShadowDistanceMin"] = fShadowDistanceMin;

	JSONSettings["fLODFadeOutMultObjectsMax"] = fLODFadeOutMultObjectsMax;
	JSONSettings["fLODFadeOutMultObjectsMin"] = fLODFadeOutMultObjectsMin;

	JSONSettings["fLODFadeOutMultItemsMax"] = fLODFadeOutMultItemsMax;
	JSONSettings["fLODFadeOutMultItemsMin"] = fLODFadeOutMultItemsMin;

	JSONSettings["fLODFadeOutMultActorsMax"] = fLODFadeOutMultActorsMax;
	JSONSettings["fLODFadeOutMultActorsMin"] = fLODFadeOutMultActorsMin;

	o << JSONSettings.dump(1);
}

void ShadowBoost::Start()
{
	init = true;
}

void ShadowBoost::UpdateShadows(float a_avgTiming)
{
	float timeRatio = a_avgTiming / (1 / (TargetFPS + 3));
	float scaleRatio = (RateOfChange / 100) * (1.0f - timeRatio) + 1.0f;

	if (ShadowsEnabled)
		fShadowDistance = std::clamp(fShadowDistance * scaleRatio, fShadowDistanceMin, fShadowDistanceMax);
	else
		fShadowDistance = fShadowDistanceMax;
	UpdateShadowDistance();

	if (LODEnabled) {
		fLODFadeOutMultObjects = std::clamp(fLODFadeOutMultObjects * scaleRatio, fLODFadeOutMultObjectsMin, fLODFadeOutMultObjectsMax);
		fLODFadeOutMultItems = std::clamp(fLODFadeOutMultItems * scaleRatio, fLODFadeOutMultItemsMin, fLODFadeOutMultItemsMax);
		fLODFadeOutMultActors = std::clamp(fLODFadeOutMultActors * scaleRatio, fLODFadeOutMultActorsMin, fLODFadeOutMultActorsMax);
	} else {
		fLODFadeOutMultObjects = fLODFadeOutMultObjectsMax;
		fLODFadeOutMultItems = fLODFadeOutMultItemsMax;
		fLODFadeOutMultActors = fLODFadeOutMultActorsMin;
	}
}

void ShadowBoost::UpdateSettings()
{
	long long frameEnd = PerformanceCounter();
	double    elapsedMilliseconds = (double)((frameEnd - frameStart)) / PerformanceFrequency();
	float     frameTime = (float)elapsedMilliseconds;
	UpdateShadows(frameTime);
	lastCPUFrameTime = frameTime;
}

void ShadowBoost::Update()
{
	if (init)
		UpdateSettings();
	frameStart = PerformanceCounter();
}

#define UI_ADD_MINMAXDISTANCE(maxparam, minparam)                                                                                                        \
	g_ENB->TwAddVarCB(bar, #maxparam, ETwType::TW_TYPE_FLOAT, UISet##maxparam, UIGet##maxparam, this, "group=MOD:ShadowBoost min=1.00 max=1000000.0"); \
	g_ENB->TwAddVarCB(bar, #minparam, ETwType::TW_TYPE_FLOAT, UISet##minparam, UIGet##minparam, this, "group=MOD:ShadowBoost min=1.00 max=1000000.0");

#define UI_ADD_CURRENTDISTANCE(param) \
	g_ENB->TwAddVarRW(bar, #param, ETwType::TW_TYPE_FLOAT, &param, "group=MOD:ShadowBoost readonly=true");

void ShadowBoost::UpdateUI()
{
	auto bar = g_ENB->TwGetBarByEnum(!REL::Module::IsVR() ? ENB_API::ENBWindowType::EditorBarEffects : ENB_API::ENBWindowType::EditorBarObjects); // ENB misnames its own bar, whoops!
	g_ENB->TwAddVarRW(bar, "Target FPS", ETwType::TW_TYPE_FLOAT, &TargetFPS, "group=MOD:ShadowBoost");
	g_ENB->TwAddVarRW(bar, "Distance Change Speed", ETwType::TW_TYPE_FLOAT, &RateOfChange, "group=MOD:ShadowBoost  min=0.00 max=1000.0");

	g_ENB->TwAddVarRW(bar, "Shadows Enabled", ETwType::TW_TYPE_BOOLCPP, &ShadowsEnabled, "group=MOD:ShadowBoost");
	UI_ADD_MINMAXDISTANCE(fShadowDistanceMax, fShadowDistanceMin)
	UI_ADD_CURRENTDISTANCE(fShadowDistance)

	g_ENB->TwAddVarRW(bar, "LOD Enabled", ETwType::TW_TYPE_BOOLCPP, &LODEnabled, "group=MOD:ShadowBoost");
	UI_ADD_MINMAXDISTANCE(fLODFadeOutMultObjectsMax, fLODFadeOutMultObjectsMin)
	UI_ADD_MINMAXDISTANCE(fLODFadeOutMultItemsMax, fLODFadeOutMultItemsMin)
	UI_ADD_MINMAXDISTANCE(fLODFadeOutMultActorsMax, fLODFadeOutMultActorsMin)
	UI_ADD_CURRENTDISTANCE(fLODFadeOutMultObjects)
	UI_ADD_CURRENTDISTANCE(fLODFadeOutMultItems)
	UI_ADD_CURRENTDISTANCE(fLODFadeOutMultActors)

	g_ENB->TwAddVarRW(bar, "Last CPU Frametime", ETwType::TW_TYPE_FLOAT, &lastCPUFrameTime, "group=MOD:ShadowBoost readonly=true");
}

#undef UI_ADD_MINMAXDISTANCE
#undef UI_ADD_CURRENTDISTANCE
