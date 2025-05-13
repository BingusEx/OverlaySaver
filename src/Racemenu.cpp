#include "Racemenu.hpp"
#include "Serialization.hpp"

namespace OverlaySaver {

	void Racemenu::Register() {

		SKEE::InterfaceExchangeMessage msg;

		const auto* const intfc{ SKSE::GetMessagingInterface() };

		intfc->Dispatch(SKEE::InterfaceExchangeMessage::kMessage_ExchangeInterface , &msg, sizeof(SKEE::InterfaceExchangeMessage*), "SKEE");

		if (!msg.interfaceMap) {
			logger::error("Couldn't Get SKSE interface map.");
			return;
		}

		OverlayInterface = static_cast<SKEE::IOverlayInterface*>(msg.interfaceMap->QueryInterface("Overlay")); // NOLINT(*-pro-type-static-cast-downcast)
		OverrideInterface = static_cast<SKEE::IOverrideInterface*>(msg.interfaceMap->QueryInterface("OVerride")); // NOLINT(*-pro-type-static-cast-downcast)

		if (!OverlayInterface) {
			logger::error("Couldn't get SKEE OverlayInterface.");
			return;
		}

		if (!OverrideInterface) {
			logger::error("Couldn't get SKEE OverrideInterface.");
			return;
		}

		logger::info("SKEE OverlayInterface Version {}", OverlayInterface->GetVersion());
		logger::info("SKEE OverrideInterface Version {}", OverrideInterface->GetVersion());

	}

	void Racemenu::OverlayManager::FlipStoredOverlaysAndReapply(RE::Actor* a_Actor) {

		auto ActorData = Serialization::GetSingleton().GetData(a_Actor);

		if (!ActorData) {
			logger::error("GetOverlaysFromRM() Actor Does not exist in map");
			return;
		}

		//doesnt work
		//OverlayInterface->RemoveOverlays(a_Actor, true);

		ClearOverlays(a_Actor);

		std::ranges::reverse(ActorData->vFeet);
		std::ranges::reverse(ActorData->vHead);
		std::ranges::reverse(ActorData->vBody);
		std::ranges::reverse(ActorData->vHand);

		ApplyOverlayFromList(a_Actor);
	}

	void Racemenu::OverlayManager::ClearBodyPart(RE::Actor* a_Actor, int a_numOfSlots, const std::string& a_OvlName) {

		if (!a_Actor) {
			logger::error("Invalid Actor");
			return;
		}

		for (int i = 0; i < a_numOfSlots; i++) {

			std::string NodeName = fmt::format(fmt::runtime(a_OvlName), i);

			Racemenu::OverlayManager::Variant VariantStr("");
			OverrideInterface->AddNodeOverride(a_Actor, static_cast<bool>(a_Actor->GetActorBase()->GetSex()), NodeName.c_str(), static_cast<uint16_t>(OverlayLayers::kShaderTexture), 0, VariantStr);

			Racemenu::OverlayManager::Variant VariantF(0.0f);
			OverrideInterface->AddNodeOverride(a_Actor, static_cast<bool>(a_Actor->GetActorBase()->GetSex()), NodeName.c_str(), static_cast<uint16_t>(OverlayLayers::kShaderAlpha), 0, VariantF);

			Racemenu::OverlayManager::Variant VariantI(0);
			OverrideInterface->AddNodeOverride(a_Actor, static_cast<bool>(a_Actor->GetActorBase()->GetSex()), NodeName.c_str(), static_cast<uint16_t>(OverlayLayers::kShaderTintColor), 0, VariantI);

		}

		logger::info("Called BodyPart Clear on {} On OvlPart {}", a_Actor->GetDisplayFullName(), a_OvlName);
	}

	void Racemenu::OverlayManager::ClearOverlays(RE::Actor* a_Actor) {

		if (!a_Actor) {
			logger::error("Invalid Actor");
			return;
		}

		ClearBodyPart(a_Actor, iHead, sHead);
		ClearBodyPart(a_Actor, iHead, sHand);
		ClearBodyPart(a_Actor, iHead, sBody);
		ClearBodyPart(a_Actor, iHead, sFeet);

		logger::info("Called ClearOverlays on {}", a_Actor->GetDisplayFullName());

	}

	void Racemenu::OverlayManager::GetNumOfSlotsFromRM() {

		iHead = static_cast<int>(OverlayInterface->GetOverlayCount(SKEE::IOverlayInterface::OverlayType::Normal, SKEE::IOverlayInterface::OverlayLocation::Face));
		iHand = static_cast<int>(OverlayInterface->GetOverlayCount(SKEE::IOverlayInterface::OverlayType::Normal, SKEE::IOverlayInterface::OverlayLocation::Hand));
		iBody = static_cast<int>(OverlayInterface->GetOverlayCount(SKEE::IOverlayInterface::OverlayType::Normal, SKEE::IOverlayInterface::OverlayLocation::Body));
		iFeet = static_cast<int>(OverlayInterface->GetOverlayCount(SKEE::IOverlayInterface::OverlayType::Normal, SKEE::IOverlayInterface::OverlayLocation::Feet));

		logger::info("RM Says There are {} Head, {} Hand, {} Body and {} Feet Ovl Slots", iHead, iHand, iBody, iFeet);
	}

	void Racemenu::OverlayManager::BuildOvlListForSlot(RE::Actor* a_Actor, int a_NumOfSlots, const std::string& a_OvlName, std::vector<RMOverlay>* a_OvlVec){

		if (!a_Actor) {
			logger::error("Invalid Actor");
			return;
		}

		for (int i = 0; i < a_NumOfSlots; i++) {
			std::string NodeName = fmt::format(fmt::runtime(a_OvlName), i);

			std::string NodeDataTextureStr = GetNodeOverrideString(a_Actor, static_cast<bool>(a_Actor->GetActorBase()->GetSex()), NodeName.c_str(), static_cast<int>(OverlayLayers::kShaderTexture), 0).c_str();

			if (NodeDataTextureStr.empty()) {
				continue;
			}

			float _Alpha = GetNodeOverrideFloat(a_Actor, static_cast<bool>(a_Actor->GetActorBase()->GetSex()), NodeName.c_str(), static_cast<int>(OverlayLayers::kShaderAlpha), -1);
			uint32_t _Color = GetNodeOverrideInt(a_Actor, static_cast<bool>(a_Actor->GetActorBase()->GetSex()), NodeName.c_str(), static_cast<int>(OverlayLayers::kShaderTintColor), -1);
			a_OvlVec->emplace_back(_Alpha, _Color, static_cast<uint32_t>(NodeDataTextureStr.length()), NodeDataTextureStr);
			
		}

		logger::info("Stored {} Ovls For BodyPart {} On Actor {}", a_OvlVec->size(), a_OvlName,a_Actor->GetDisplayFullName());

	}

	void Racemenu::OverlayManager::BuildOverlayList(RE::Actor* a_Actor) {

		if (!a_Actor) {
			logger::error("GetOverlaysFromRM() Actor Was null");
			return;
		}

		auto ActorData = Serialization::GetSingleton().GetData(a_Actor);

		if (!ActorData) {
			logger::error("GetOverlaysFromRM() Actor Does not exist in map");
			return;
		}

		ActorData->vFeet.clear();
		ActorData->vHead.clear();
		ActorData->vBody.clear();
		ActorData->vHand.clear();

		BuildOvlListForSlot(a_Actor, iHead, sHead, &ActorData->vHead);
		BuildOvlListForSlot(a_Actor, iHand, sHand, &ActorData->vHand);
		BuildOvlListForSlot(a_Actor, iBody, sBody, &ActorData->vBody);
		BuildOvlListForSlot(a_Actor, iFeet, sFeet, &ActorData->vFeet);

		logger::info("GetOverlaysFromRM() Completed on Actor {}", a_Actor->GetDisplayFullName());
		logger::info("{} Head Overlays Added", ActorData->vHead.size());
		logger::info("{} Hand Overlays Added", ActorData->vHand.size());
		logger::info("{} Body Overlays Added", ActorData->vBody.size());
		logger::info("{} Feet Overlays Added", ActorData->vFeet.size());

	}

	void Racemenu::OverlayManager::ApplyStoredOverlayOnBodyPart(RE::Actor* a_Actor, int a_NumOfSlots, const std::string& a_OvlName, std::vector<RMOverlay>* a_OvlVec) {


		if (a_OvlVec->empty()) {
			logger::info("Vector was empty, Skipping");
			return;
		}

		const int NumOfSlots = std::min(a_NumOfSlots, static_cast<int>(a_OvlVec->size()));

		for (int i = 0; i < NumOfSlots; i++) {
			std::string NodeName = fmt::format(fmt::runtime(a_OvlName),i);

			try {
				Variant VariantStr(a_OvlVec->at(i).TexturePath.c_str());
				OverrideInterface->AddNodeOverride(a_Actor, static_cast<bool>(a_Actor->GetActorBase()->GetSex()), NodeName.c_str(), static_cast<uint16_t>(OverlayLayers::kShaderTexture), 0, VariantStr);
			}
			catch (const std::exception&) {}

			try {
				Variant VariantF(a_OvlVec->at(i).Alpha);
				OverrideInterface->AddNodeOverride(a_Actor, static_cast<bool>(a_Actor->GetActorBase()->GetSex()), NodeName.c_str(), static_cast<uint16_t>(OverlayLayers::kShaderAlpha), 0, VariantF);
			}
			catch (const std::exception&) {}

			try {
				Variant VariantI(static_cast<int32_t>(a_OvlVec->at(i).Color));
				OverrideInterface->AddNodeOverride(a_Actor, static_cast<bool>(a_Actor->GetActorBase()->GetSex()), NodeName.c_str(), static_cast<uint16_t>(OverlayLayers::kShaderTintColor), 0, VariantI);
			}
			catch (const std::exception&) {}
		}

		logger::info("Appied {} Ovls From Persistent Storage to {} on BodyPart {}", a_NumOfSlots, a_Actor->GetDisplayFullName(), a_OvlName);

	}

	void Racemenu::OverlayManager::ApplyOverlayFromList(RE::Actor* a_Actor) {

		if (!a_Actor) {
			//logger::error("ApplyOverlayFromList() Actor Was null");
			return;
		}

		auto ActorData = Serialization::GetSingleton().GetData(a_Actor);
		if (!ActorData) {
			//logger::error("ApplyOverlayFromList() Actor Does not exist in map");
			return;
		}

		ApplyStoredOverlayOnBodyPart(a_Actor, iHead, sHead, &ActorData->vHead);
		ApplyStoredOverlayOnBodyPart(a_Actor, iHand, sHand, &ActorData->vHand);
		ApplyStoredOverlayOnBodyPart(a_Actor, iBody, sBody, &ActorData->vBody);
		ApplyStoredOverlayOnBodyPart(a_Actor, iFeet, sFeet, &ActorData->vFeet);

		logger::info("ApplyOverlayFromList() On Actor {} OK", a_Actor->GetDisplayFullName());

		if (const auto& Actor3D = a_Actor->GetCurrent3D()) {
			OverrideInterface->ApplyNodeOverrides(a_Actor, Actor3D, true);
			a_Actor->Update3DModel();
		}

		logger::info("GetOverlaysFromRM() Has Run");

	}

}