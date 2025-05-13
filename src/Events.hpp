#pragma once
#include "Serialization.hpp"

namespace OverlaySaver {

	//Taken From Po3 Tweaks

	struct Load3D {

		using Target = RE::Character;
		static inline constexpr std::size_t index{ 0x6A };

		static RE::NiAVObject* thunk(RE::Character* actor, bool a_backgroundLoading) {

			NiAVObject* Res = func(actor, a_backgroundLoading);

			const auto& intfc = SKSE::GetTaskInterface();
			intfc->AddTask([actor] {
				Racemenu::OverlayManager::ApplyOverlayFromList(actor);
			});

			return Res;

		}

		static inline void post_hook() {
			logger::info("Installed Load3D hook.");
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};
}
