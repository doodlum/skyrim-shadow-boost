#pragma once

namespace Events
{
	void Register();
}

//class cellLoadEventHandler : public RE::BSTEventSink<RE::TESCellFullyLoadedEvent>
//{
//public:
//	virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESCellFullyLoadedEvent* a_event, RE::BSTEventSource<RE::TESCellFullyLoadedEvent>* a_eventSource);
//
//	static bool Register()
//	{
//		static cellLoadEventHandler singleton;
//		auto                        ScriptEventSource = RE::ScriptEventSourceHolder::GetSingleton();
//
//		if (!ScriptEventSource) {
//			logger::error("Script event source not found");
//			return false;
//		}
//
//		ScriptEventSource->AddEventSink(&singleton);
//
//		logger::info("Registered {}", typeid(singleton).name());
//
//		return true;
//	}
//};
