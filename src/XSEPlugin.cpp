#include "ShadowBoost.h"


void PatchD3D11();


void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		ShadowBoost::GetSingleton()->g_ENB = reinterpret_cast<ENB_API::ENBSDKALT1001*>(ENB_API::RequestENBAPI(ENB_API::SDKVersion::V1001));
		if (ShadowBoost::GetSingleton()->g_ENB) {
			logger::info("Obtained ENB API");
			ShadowBoost::GetSingleton()->g_ENB->SetCallbackFunction([](ENBCallbackType calltype) {
				switch (calltype) {
				case ENBCallbackType::ENBCallback_PostLoad:
					ShadowBoost::GetSingleton()->LoadINI();
					ShadowBoost::GetSingleton()->RefreshUI();
					break;
				case ENBCallbackType::ENBCallback_PreSave:
					ShadowBoost::GetSingleton()->SaveINI();
					break;
				}
			});
		} else
			logger::info("Unable to acquire ENB API");

		break;

	case SKSE::MessagingInterface::kDataLoaded:
		ShadowBoost::GetSingleton()->Start();
		if (!GetModuleHandle(L"Data\\SKSE\\Plugins\\SSEDisplayTweaks.dll")) {
			RE::DebugMessageBox("Warning: ShadowBoost requires SSE Display Tweaks");
		}
		break;
	}
}

void Load()
{
	PatchD3D11();
	ShadowBoost::InstallHooks();
	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
}