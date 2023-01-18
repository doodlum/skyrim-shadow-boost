#include "ShadowBoost.h"

#include <SimpleIni.h>

float& ShadowBoost::GetGameSettingFloat(std::string a_name, std::string a_section)
{
	auto ini = RE::INISettingCollection::GetSingleton();
	return ini->GetSetting(std::format("{}:{}", a_name, a_section))->data.f;
}

#define GetSettingFloat(a_section, a_setting) a_setting = (float)ini.GetDoubleValue(a_section, #a_setting, 1.0f);
#define SetSettingFloat(a_section, a_setting) ini.SetDoubleValue(a_section, #a_setting, a_setting);

#define GetSettingBool(a_section, a_setting) a_setting = ini.GetBoolValue(a_section, #a_setting, true);
#define SetSettingBool(a_section, a_setting) ini.SetBoolValue(a_section, #a_setting, a_setting);

void ShadowBoost::LoadINI()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);
	CSimpleIniA                        ini;
	ini.SetUnicode();
	ini.LoadFile(L"Data\\SKSE\\Plugins\\ShadowBoost.ini");

	GetSettingFloat("Settings", fTargetFPS);
	GetSettingFloat("Settings", fRateOfChange);
	
	GetSettingBool("Shadows", bShadowsEnabled);

	GetSettingFloat("Shadows", fShadowDistanceMax);
	GetSettingFloat("Shadows", fShadowDistanceMin);

	GetSettingBool("LOD", bLODEnabled);

	GetSettingFloat("LOD", fLODFadeOutMultObjectsMax);
	GetSettingFloat("LOD", fLODFadeOutMultObjectsMin);

	GetSettingFloat("LOD", fLODFadeOutMultItemsMax);
	GetSettingFloat("LOD", fLODFadeOutMultItemsMin);

	GetSettingFloat("LOD", fLODFadeOutMultActorsMax);
	GetSettingFloat("LOD", fLODFadeOutMultActorsMin);

	GetSettingBool("Terrain", bTerrainEnabled);

	GetSettingFloat("Terrain", fBlockLevel0DistanceMax);
	GetSettingFloat("Terrain", fBlockLevel0DistanceMin);

	GetSettingFloat("Terrain", fBlockLevel1DistanceMax);
	GetSettingFloat("Terrain", fBlockLevel1DistanceMin);

	GetSettingFloat("Terrain", fTreeLoadDistanceMax);
	GetSettingFloat("Terrain", fTreeLoadDistanceMin);
}

void ShadowBoost::SaveINI()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);
	CSimpleIniA                        ini;
	ini.SetUnicode();
	
	SetSettingFloat("Settings", fTargetFPS);
	SetSettingFloat("Settings", fRateOfChange);
	
	SetSettingBool("Shadows", bShadowsEnabled);

	SetSettingFloat("Shadows", fShadowDistanceMax);
	SetSettingFloat("Shadows", fShadowDistanceMin);

	SetSettingBool("LOD", bLODEnabled);

	SetSettingFloat("LOD", fLODFadeOutMultObjectsMax);
	SetSettingFloat("LOD", fLODFadeOutMultObjectsMin);

	SetSettingFloat("LOD", fLODFadeOutMultItemsMax);
	SetSettingFloat("LOD", fLODFadeOutMultItemsMin);

	SetSettingFloat("LOD", fLODFadeOutMultActorsMax);
	SetSettingFloat("LOD", fLODFadeOutMultActorsMin);

	SetSettingBool("Terrain", bTerrainEnabled);

	SetSettingFloat("Terrain", fBlockLevel0DistanceMax);
	SetSettingFloat("Terrain", fBlockLevel0DistanceMin);

	SetSettingFloat("Terrain", fBlockLevel1DistanceMax);
	SetSettingFloat("Terrain", fBlockLevel1DistanceMin);

	SetSettingFloat("Terrain", fTreeLoadDistanceMax);
	SetSettingFloat("Terrain", fTreeLoadDistanceMin);

	ini.SaveFile(L"Data\\SKSE\\Plugins\\ShadowBoost.ini");
}

void ShadowBoost::Start()
{
	init = true;
}

void ShadowBoost::UpdateShadows(float a_avgTiming)
{
	static float& fShadowDistance = (*(float*)RELOCATION_ID(513793, 391845).address());
	static float& fLODFadeOutMultObjects = (*(float*)RELOCATION_ID(500935, 358960).address());
	static float& fLODFadeOutMultItems = (*(float*)RELOCATION_ID(500933, 358957).address());
	static float& fLODFadeOutMultActors = (*(float*)RELOCATION_ID(500931, 358954).address());
	static float& fBlockLevel0Distance = (*(float*)RELOCATION_ID(508313, 379925).address());
	static float& fBlockLevel1Distance = (*(float*)RELOCATION_ID(508315, 379928).address());
	static float& fTreeLoadDistance = (*(float*)RELOCATION_ID(508319, 379934).address());

	float timeRatio = a_avgTiming / (1 / (fTargetFPS + 3));
	float scaleRatio = (fRateOfChange / 100) * (1.0f - timeRatio) + 1.0f;

	if (bShadowsEnabled)
		fShadowDistance = std::clamp(fShadowDistance * scaleRatio, fShadowDistanceMin, fShadowDistanceMax);
	else
		fShadowDistance = fShadowDistanceMax;
	UpdateShadowDistance();

	if (bLODEnabled) {
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

#define UI_ADD_MINMAXDISTANCE(param)                                                                                                        \
	g_ENB->TwAddVarCB(bar, #param "Max", ETwType::TW_TYPE_FLOAT, UISet##param##Max, UIGet##param##Max, this, "group=MOD:ShadowBoost min=1.00 max=1000000.0"); \
	g_ENB->TwAddVarCB(bar, #param "Min", ETwType::TW_TYPE_FLOAT, UISet##param##Min, UIGet##param##Min, this, "group=MOD:ShadowBoost min=1.00 max=1000000.0");

#define UI_ADD_CURRENTDISTANCE(param) \
	g_ENB->TwAddVarRW(bar, #param, ETwType::TW_TYPE_FLOAT, &param, "group=MOD:ShadowBoost readonly=true");

void ShadowBoost::RefreshUI()
{
	static float& fShadowDistance = (*(float*)RELOCATION_ID(513793, 391845).address());
	static float& fLODFadeOutMultObjects = (*(float*)RELOCATION_ID(500935, 358960).address());
	static float& fLODFadeOutMultItems = (*(float*)RELOCATION_ID(500933, 358957).address());
	static float& fLODFadeOutMultActors = (*(float*)RELOCATION_ID(500931, 358954).address());
	static float& fBlockLevel0Distance = (*(float*)RELOCATION_ID(508313, 379925).address());
	static float& fBlockLevel1Distance = (*(float*)RELOCATION_ID(508315, 379928).address());
	static float& fTreeLoadDistance = (*(float*)RELOCATION_ID(508319, 379934).address());

	auto bar = g_ENB->TwGetBarByEnum(!REL::Module::IsVR() ? ENB_API::ENBWindowType::EditorBarEffects : ENB_API::ENBWindowType::EditorBarObjects); // ENB misnames its own bar, whoops!
	g_ENB->TwAddVarRW(bar, "Target FPS", ETwType::TW_TYPE_FLOAT, &fTargetFPS, "group=MOD:ShadowBoost");
	g_ENB->TwAddVarRW(bar, "Distance Change Speed", ETwType::TW_TYPE_FLOAT, &fRateOfChange, "group=MOD:ShadowBoost  min=0.00 max=1000.0");

	g_ENB->TwAddVarRW(bar, "Shadows Enabled", ETwType::TW_TYPE_BOOLCPP, &bShadowsEnabled, "group=MOD:ShadowBoost");
	UI_ADD_MINMAXDISTANCE(fShadowDistance)
	UI_ADD_CURRENTDISTANCE(fShadowDistance)

	g_ENB->TwAddVarRW(bar, "LOD Enabled", ETwType::TW_TYPE_BOOLCPP, &bLODEnabled, "group=MOD:ShadowBoost");
	UI_ADD_MINMAXDISTANCE(fLODFadeOutMultObjects)
	UI_ADD_MINMAXDISTANCE(fLODFadeOutMultItems)
	UI_ADD_MINMAXDISTANCE(fLODFadeOutMultActors)
	UI_ADD_CURRENTDISTANCE(fLODFadeOutMultObjects)
	UI_ADD_CURRENTDISTANCE(fLODFadeOutMultItems)
	UI_ADD_CURRENTDISTANCE(fLODFadeOutMultActors)

	g_ENB->TwAddVarRW(bar, "Terrain Enabled", ETwType::TW_TYPE_BOOLCPP, &bTerrainEnabled, "group=MOD:ShadowBoost");
	UI_ADD_MINMAXDISTANCE(fBlockLevel0Distance)
	UI_ADD_MINMAXDISTANCE(fBlockLevel1Distance)
	UI_ADD_MINMAXDISTANCE(fLODFadeOutMultActors)
	UI_ADD_CURRENTDISTANCE(fBlockLevel0Distance)
	UI_ADD_CURRENTDISTANCE(fBlockLevel1Distance)
	UI_ADD_CURRENTDISTANCE(fTreeLoadDistance)

	g_ENB->TwAddVarRW(bar, "Last CPU Frametime", ETwType::TW_TYPE_FLOAT, &lastCPUFrameTime, "group=MOD:ShadowBoost readonly=true");
}

#undef UI_ADD_MINMAXDISTANCE
#undef UI_ADD_CURRENTDISTANCE
