#include "Serialization.hpp"

namespace OverlaySaver {

	void Serialization::OnGameLoaded(SerializationInterface* serde) {

		std::unique_lock lock(GetSingleton()._Lock);

		GetSingleton().ActorData.clear();

		std::uint32_t RecordType;
		std::uint32_t RecordSize;
		std::uint32_t RecordVersion;

		while (serde->GetNextRecordInfo(RecordType, RecordVersion, RecordSize)) {
			LoadActorData(serde, RecordType, RecordVersion);
		}
	}

	void Serialization::OnGameSaved(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._Lock);

		for (const auto ActorFormID : GetSingleton().ActorData | std::views::keys) {

			if (const auto Act = TESForm::LookupByID<Actor>(ActorFormID)) {
				Racemenu::OverlayManager::GetSingleton().BuildOverlayList(Act);
				logger::debug("Built OvlList for Actor {} Before Save", Act->GetDisplayFullName());
			}

		}

		SaveActorData(serde, OverlayStructVersion);
	}

	void Serialization::LoadActorData(SKSE::SerializationInterface* serde, const uint32_t RecordType, const uint32_t RecordVersion) {

		if (RecordType != ActorDataRecord) {
			return;
		}

		if (RecordVersion < 1) {
			return;
		}

		uint32_t NumOfActorRecords = 0;
		serde->ReadRecordData(&NumOfActorRecords, sizeof(NumOfActorRecords));

		if (NumOfActorRecords <= 0) {
			logger::debug("No data to load");
			return;
		}

		for (; NumOfActorRecords > 0; --NumOfActorRecords) {

			FormID ActorRefID;
			serde->ReadRecordData(&ActorRefID, sizeof(ActorRefID));

			uint8_t NumOfHeadOvl = 0;
			serde->ReadRecordData(&NumOfHeadOvl, sizeof(NumOfHeadOvl));

			uint8_t NumOfHandsOvl = 0;
			serde->ReadRecordData(&NumOfHandsOvl, sizeof(NumOfHandsOvl));

			uint8_t NumOfBodydOvl = 0;
			serde->ReadRecordData(&NumOfBodydOvl, sizeof(NumOfBodydOvl));

			uint8_t NumOfFeetOvl = 0;
			serde->ReadRecordData(&NumOfFeetOvl, sizeof(NumOfFeetOvl));

			ActorRecord TempDat = {};

			//------ Head
			for (; NumOfHeadOvl > 0; --NumOfHeadOvl) {

				RMOverlay TmpOvl = {};

				serde->ReadRecordData(&TmpOvl.Alpha, sizeof(TmpOvl.Alpha));
				serde->ReadRecordData(&TmpOvl.Color, sizeof(TmpOvl.Color));
				serde->ReadRecordData(&TmpOvl.TexStrLen, sizeof(TmpOvl.TexStrLen));

				TmpOvl.TexturePath.resize(TmpOvl.TexStrLen + 1);
				serde->ReadRecordData(TmpOvl.TexturePath.data(), TmpOvl.TexStrLen);
				TmpOvl.TexturePath[TmpOvl.TexStrLen] = '\0';

				TempDat.vHead.emplace_back(TmpOvl);

			}

			//------ Hands

			for (; NumOfHandsOvl > 0; --NumOfHandsOvl) {

				RMOverlay TmpOvl = {};

				serde->ReadRecordData(&TmpOvl.Alpha, sizeof(TmpOvl.Alpha));
				serde->ReadRecordData(&TmpOvl.Color, sizeof(TmpOvl.Color));
				serde->ReadRecordData(&TmpOvl.TexStrLen, sizeof(TmpOvl.TexStrLen));

				TmpOvl.TexturePath.resize(TmpOvl.TexStrLen + 1);
				serde->ReadRecordData(TmpOvl.TexturePath.data(), TmpOvl.TexStrLen);
				TmpOvl.TexturePath[TmpOvl.TexStrLen] = '\0';

				TempDat.vHand.emplace_back(TmpOvl);

			}

			//------ Body

			for (; NumOfBodydOvl > 0; --NumOfBodydOvl) {

				RMOverlay TmpOvl = {};

				serde->ReadRecordData(&TmpOvl.Alpha, sizeof(TmpOvl.Alpha));
				serde->ReadRecordData(&TmpOvl.Color, sizeof(TmpOvl.Color));
				serde->ReadRecordData(&TmpOvl.TexStrLen, sizeof(TmpOvl.TexStrLen));

				TmpOvl.TexturePath.resize(TmpOvl.TexStrLen + 1);
				serde->ReadRecordData(TmpOvl.TexturePath.data(), TmpOvl.TexStrLen);
				TmpOvl.TexturePath[TmpOvl.TexStrLen] = '\0';

				TempDat.vBody.emplace_back(TmpOvl);

			}

			//------ Feet

			for (; NumOfFeetOvl > 0; --NumOfFeetOvl) {

				RMOverlay TmpOvl = {};

				serde->ReadRecordData(&TmpOvl.Alpha, sizeof(TmpOvl.Alpha));
				serde->ReadRecordData(&TmpOvl.Color, sizeof(TmpOvl.Color));
				serde->ReadRecordData(&TmpOvl.TexStrLen, sizeof(TmpOvl.TexStrLen));

				TmpOvl.TexturePath.resize(TmpOvl.TexStrLen + 1);
				serde->ReadRecordData(TmpOvl.TexturePath.data(), TmpOvl.TexStrLen);
				TmpOvl.TexturePath[TmpOvl.TexStrLen] = '\0';

				TempDat.vFeet.emplace_back(TmpOvl);

			}


			RE::FormID CorrectedFormID;
			if (serde->ResolveFormID(ActorRefID, CorrectedFormID)) {

				log::info("LoadActorData() Actor Persistent data loaded for FormID {:08X}", ActorRefID);

				if (Actor* ActorForm = TESForm::LookupByID<Actor>(CorrectedFormID)) {
					if (ActorForm) {
						Serialization::GetSingleton().ActorData.insert_or_assign(CorrectedFormID, TempDat);
					}
				}
				else {
					log::warn("LoadActorData() Actor FormID {:08X} could not be found after loading the save.", CorrectedFormID);
					Serialization::GetSingleton().ActorData.erase(CorrectedFormID);
				}
			}
			else {
				log::warn("LoadActorData() Actor FormID {:08X} could not be resolved. Not adding to ActorDataMap.", ActorRefID);
			}

		}

	}

	void Serialization::SaveActorData(SKSE::SerializationInterface* serde, const uint8_t Version) {

		const uint32_t NumOfActorRecords = GetSingleton().ActorData.size();

		if (NumOfActorRecords <= 0) {
			logger::debug("No data to save");
		}

		if (!serde->OpenRecord(ActorDataRecord, Version)) {
			log::critical("Unable to open ActorDataRecord in CoSave.");
			return;
		}

		serde->WriteRecordData(&NumOfActorRecords, sizeof(NumOfActorRecords));

		for (auto const& [ActorFormID, Data] : GetSingleton().ActorData) {


			serde->WriteRecordData(&ActorFormID, sizeof(ActorFormID));

			uint8_t NumOfHeadOvl = Data.vHead.size();
			serde->WriteRecordData(&NumOfHeadOvl, sizeof(NumOfHeadOvl));

			uint8_t NumOfHandsOvl = Data.vHand.size();
			serde->WriteRecordData(&NumOfHandsOvl, sizeof(NumOfHandsOvl));

			uint8_t NumOfBodydOvl = Data.vBody.size();
			serde->WriteRecordData(&NumOfBodydOvl, sizeof(NumOfBodydOvl));

			uint8_t NumOfFeetOvl = Data.vFeet.size();
			serde->WriteRecordData(&NumOfFeetOvl, sizeof(NumOfFeetOvl));

			logger::debug("NumOfHeadOvl: {}", NumOfHeadOvl);
			logger::debug("NumOfHandsOvl: {}", NumOfHandsOvl);
			logger::debug("NumOfBodydOvl: {}", NumOfBodydOvl);
			logger::debug("NumOfFeetOvl: {}", NumOfFeetOvl);

			//----- Head

			for (auto& OvlData : Data.vHead) {

				serde->WriteRecordData(&OvlData.Alpha, sizeof(OvlData.Alpha));
				serde->WriteRecordData(&OvlData.Color, sizeof(OvlData.Color));

				const uint16_t TextStrLen = OvlData.TexturePath.length();
				serde->WriteRecordData(&TextStrLen, sizeof(uint16_t));
				serde->WriteRecordData(OvlData.TexturePath.data(), TextStrLen);

			}

			for (auto& OvlData : Data.vHand) {

				serde->WriteRecordData(&OvlData.Alpha, sizeof(OvlData.Alpha));
				serde->WriteRecordData(&OvlData.Color, sizeof(OvlData.Color));

				const uint16_t TextStrLen = OvlData.TexturePath.length();
				serde->WriteRecordData(&TextStrLen, sizeof(uint16_t));
				serde->WriteRecordData(OvlData.TexturePath.data(), TextStrLen);

			}

			for (auto& OvlData : Data.vBody) {

				serde->WriteRecordData(&OvlData.Alpha, sizeof(OvlData.Alpha));
				serde->WriteRecordData(&OvlData.Color, sizeof(OvlData.Color));

				const uint16_t TextStrLen = OvlData.TexturePath.length();
				serde->WriteRecordData(&TextStrLen, sizeof(uint16_t));
				serde->WriteRecordData(OvlData.TexturePath.data(), TextStrLen);

			}

			for (auto& OvlData : Data.vFeet) {

				serde->WriteRecordData(&OvlData.Alpha, sizeof(OvlData.Alpha));
				serde->WriteRecordData(&OvlData.Color, sizeof(OvlData.Color));

				const uint16_t TextStrLen = OvlData.TexturePath.length();
				serde->WriteRecordData(&TextStrLen, sizeof(uint16_t));
				serde->WriteRecordData(OvlData.TexturePath.data(), TextStrLen);

			}
			log::info("Data serialized for Actor FormID {:08X}", ActorFormID);
		}

	}

	void Serialization::OnRevert(SerializationInterface* a_intfc) {
		GetSingleton().ClearData();
	}

	Serialization::ActorRecord* Serialization::GetData(Actor* refr) {
		//std::unique_lock lock(_Lock);

		auto key = refr->formID;

		if (!refr) {
			logger::warn("GetData() Actor is null");
			return nullptr;
		}

		try {
			if (!this->ActorData.contains(key)) {
				return nullptr;
			}
			return &this->ActorData.at(key);
		}
		catch (const std::out_of_range&) {
			return nullptr;
		}
	}

	void Serialization::AddNew(Actor* actor) {
		//std::unique_lock lock(_Lock);

		if (!actor) {
			logger::error("Tried to add a null actor");
			return;
		}

		auto key = actor->formID;
		this->ActorData.try_emplace(key, actor);
	}

	void Serialization::Erase(Actor* actor) {
		//std::unique_lock lock(_Lock);

		if (!actor) {
			logger::error("Tried to erase a null actor");
			return;
		}

		auto key = actor->formID;
		this->ActorData.erase(key);
	}

	void Serialization::ClearData() {
		std::unique_lock lock(_Lock);
		ActorData.clear();
	}

}
