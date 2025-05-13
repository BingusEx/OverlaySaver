#include "Console.hpp"
#include "Serialization.hpp"

namespace {

	template< typename ... Args >
	void Cprint(std::string_view rt_fmt_str, Args&&... args) {
		try {
			ConsoleLog::GetSingleton()->Print("%s", std::vformat(rt_fmt_str, std::make_format_args(args ...)).c_str());
		}
		catch (const std::format_error& e) {
			log::info("Could not format console log, check valid format string: {}", e.what());
		}
	}

	std::string str_tolower(std::string s) {
		std::ranges::transform(s, s.begin(), [](unsigned char c) {
			return std::tolower(c);
			});
		return s;
	}

	// Trims whitespace from the beginning and end of the string
	std::string trim(const std::string& s) {
		auto start = s.begin();
		while (start != s.end() && std::isspace(static_cast<unsigned char>(*start))) {
			++start;
		}

		auto end = s.end();
		do {
			--end;
		}
		while (end != start && std::isspace(static_cast<unsigned char>(*end)));

		return std::string(start, end + 1);
	}

	static void Thunk(RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef);
	static inline REL::Relocation<decltype(Thunk)> _ConsoleSub;


	void Thunk(RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef) {

		const std::string Input = a_script->text;
		logger::info("Entered Console Text: \"{}\"", Input);

		//If true command was handled by the plugin
		if (OverlaySaver::ConsoleManager::Process(Input)) {
			return;
		}

		_ConsoleSub(a_script, a_compiler, a_name, a_targetRef);
	}

}

namespace OverlaySaver {

	void ConsoleManager::InstallHook() {

		//SE Offset: 1408daf02
		REL::Relocation<std::uintptr_t> hook(RELOCATION_ID(52065, 52952), REL::VariantOffset(0xE2, 0x52, 0xE2).offset());

		SKSE::AllocTrampoline(14);
		_ConsoleSub = SKSE::GetTrampoline().write_call<5>(hook.address(), Thunk);
		log::info("Hooked ConsoleSub");

	}

	void ConsoleManager::RegisterCommand(std::string_view a_cmdName, const std::function<void()>& a_callback, const std::string& a_desc) {

		auto& me = GetSingleton();

		std::string name(a_cmdName);

		me.RegisteredCommands.try_emplace(name, a_callback, a_desc);

		log::info("Registered Console Command \"{} {}\"", me.Default_Preffix, name);
	}

	bool ConsoleManager::Process(const std::string& a_msg) {

		auto& me = GetSingleton();

		[[unlikely]] if (me.RegisteredCommands.empty()) return false;

		std::stringstream Msg(trim(str_tolower(a_msg)));

		std::vector<std::string> Args{};
		std::string TmpArg;

		while (Msg >> TmpArg) {

			//If subcommands are ever needed just increase this value
			if (Args.size() == 2) {
				break;
			}

			Args.emplace_back(TmpArg);

			//no "gts" ? then its not our problem to deal with
			if (Args.at(0) != me.Default_Preffix) {
				return false;
			}
		}

		//if 1 arg show help
		if (Args.size() < 2) {
			CMD_Help();
			return true;
		}

		for (const auto& registered_command : me.RegisteredCommands) {
			if (registered_command.first == Args.at(1)) {
				if (registered_command.second.callback) {
					registered_command.second.callback();
					return true;
				}
				else {
					logger::warn("Command {} has no function assigned to it", registered_command.first);
					return false;
				}
			}
		}

		Cprint("Command not found type {} help for a list of commands.", me.Default_Preffix);
		return true;
	}

	void ConsoleManager::CMD_Help() {
		auto& me = GetSingleton();
		Cprint("--- List of available commands ---");

		for (const auto& key : me.RegisteredCommands) {
			Cprint("* {} {} - {} ", me.Default_Preffix, key.first, key.second.desc);
		}
	}

	void ConsoleManager::CMD_Register() {
		if (const auto& PickData = RE::Console::GetSelectedRef()) {
			if (const auto& TaretHandle = PickData.get()) {
				if (const auto& TargetActor = skyrim_cast<Actor*>(TaretHandle)) {
					if (Serialization::GetSingleton().GetData(TargetActor)) {
						Cprint("OverlaySaver: Actor {} ({:X}) Already Exists in Map", TargetActor->GetDisplayFullName(), TargetActor->formID);
						return;
					}

					Serialization::GetSingleton().AddNew(TargetActor);
					Racemenu::OverlayManager::BuildOverlayList(TargetActor);
					Cprint("OverlaySaver: Registered {} ({:X})", TargetActor->GetDisplayFullName(), TargetActor->formID);
				}
			}
		}
	}

	void ConsoleManager::CMD_ReApply() {
		if (const auto& PickData = RE::Console::GetSelectedRef()) {
			if (const auto& TaretHandle = PickData.get()) {
				if (const auto& TargetActor = skyrim_cast<Actor*>(TaretHandle)) {
					if (Serialization::GetSingleton().GetData(TargetActor)) {
						Racemenu::OverlayManager::ApplyOverlayFromList(TargetActor);
						Cprint("OverlaySaver: Applied Saved {} ({:X})", TargetActor->GetDisplayFullName(), TargetActor->formID);
					}
				}
			}
		}
	}

	void ConsoleManager::CMD_Update() {
		if (const auto& PickData = RE::Console::GetSelectedRef()) {
			if (const auto& TaretHandle = PickData.get()) {
				if (const auto& TargetActor = skyrim_cast<Actor*>(TaretHandle)) {
					if (Serialization::GetSingleton().GetData(TargetActor)) {
						Racemenu::OverlayManager::BuildOverlayList(TargetActor);
						Cprint("OverlaySaver: Built List {} ({:X})", TargetActor->GetDisplayFullName(), TargetActor->formID);
					}
				}
			}
		}
	}

	void ConsoleManager::CMD_Erase() {
		if (const auto& PickData = RE::Console::GetSelectedRef()) {
			if (const auto& TaretHandle = PickData.get()) {
				if (const auto& TargetActor = skyrim_cast<Actor*>(TaretHandle)) {
					if (Serialization::GetSingleton().GetData(TargetActor)) {
						Serialization::GetSingleton().Erase(TargetActor);
						Cprint("OverlaySaver: Erased CosaveData from {} ({:X})", TargetActor->GetDisplayFullName(), TargetActor->formID);
					}
				}
			}
		}
	}

	void ConsoleManager::CMD_Flip() {
		if (const auto& PickData = RE::Console::GetSelectedRef()) {
			if (const auto& TaretHandle = PickData.get()) {
				if (const auto& TargetActor = skyrim_cast<Actor*>(TaretHandle)) {
					if (Serialization::GetSingleton().GetData(TargetActor)) {
						Racemenu::OverlayManager::FlipStoredOverlaysAndReapply(TargetActor);
						Cprint("OverlaySaver: Flipped Ovl order of {} ({:X})", TargetActor->GetDisplayFullName(), TargetActor->formID);
					}
				}
			}
		}
	}
}