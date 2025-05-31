
#include "Console.hpp"
#include "Events.hpp"
#include "Serialization.hpp"
#include "Hook.hpp"
#include "Racemenu.hpp"

using namespace OverlaySaver;

namespace {

	void InitializeMessaging() {

		if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
			switch (message->type) {

				case MessagingInterface::kPostPostLoad: {
					ConsoleManager::Init();
					Racemenu::Register();
					Racemenu::OverlayManager::GetNumOfSlotsFromRM();
					
					break;
				}
				default: {}
			}
			})) {
			SKSE::stl::report_and_fail("Unable to register message listener.");
		}
	}


	void InitializeSerialization() {
		auto* serde = GetSerializationInterface();
		serde->SetUniqueID(_byteswap_ulong('OVLS'));

		serde->SetSaveCallback(OverlaySaver::Serialization::OnGameSaved);
		serde->SetRevertCallback(OverlaySaver::Serialization::OnRevert);
		serde->SetLoadCallback(OverlaySaver::Serialization::OnGameLoaded);

		log::info("Cosave serialization initialized.");
	}

}


SKSEPluginLoad(const LoadInterface * a_skse) {
	Init(a_skse);

	InitializeMessaging();
	InitializeSerialization();
	ConsoleManager::InstallHook();
	Hook::stl::install_hook<OverlaySaver::Load3D>();
	spdlog::set_level(spdlog::level::err);
	return true;
}

SKSEPluginInfo(
	.Version = REL::Version{ 1, 0, 0, 0 },
	.Name = "OverlaySaver",
	.Author = "BingusEx",
	.StructCompatibility = SKSE::StructCompatibility::Independent,
	.RuntimeCompatibility = { RUNTIME_SSE_1_6_1170, RUNTIME_SSE_1_6_1130 }
);


