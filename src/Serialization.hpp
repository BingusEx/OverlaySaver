#pragma once
#include "Racemenu.hpp"

namespace OverlaySaver {

	class Serialization {

		private:

		Serialization() = default;

		constexpr static inline uint8_t OverlayStructVersion = 1;
		const static inline uint32_t ActorDataRecord = _byteswap_ulong('ACTD');

		mutable std::mutex _Lock;

		public:

		struct ActorRecord {
			std::vector<RMOverlay> vHead = {};
			std::vector<RMOverlay> vHand = {};
			std::vector<RMOverlay> vBody = {};
			std::vector<RMOverlay> vFeet = {};

			volatile bool Applied = false;

			ActorRecord() = default;
			ActorRecord(RE::Actor* actor) {}
		};

		std::unordered_map<FormID, ActorRecord> ActorData = {};


		[[nodiscard]] inline static Serialization& GetSingleton() noexcept {
			static Serialization Instance;
			return Instance;
		}

		void ClearData();
		ActorRecord* GetData(Actor* refr);
		void AddNew(Actor* actor);
		void Erase(Actor* actor);

		static void OnGameLoaded(SerializationInterface* serde);
		static void OnGameSaved(SerializationInterface* serde);
		static void LoadActorData(SKSE::SerializationInterface* serde, uint32_t RecordType, uint32_t RecordVersion);
		static void SaveActorData(SKSE::SerializationInterface* serde, uint8_t Version);
		static void OnRevert(SerializationInterface* a_intfc);

	};


}