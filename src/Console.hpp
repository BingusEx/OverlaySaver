#pragma once

namespace OverlaySaver {

    struct Command {
        std::function<void()> callback = nullptr;
        std::string desc;
        explicit Command(const std::function<void()>& callback, std::string desc) :
            callback(callback),
            desc(std::move(desc)) {}
    };

    class ConsoleManager {

        private:
        //default base command preffix
        const std::string Default_Preffix = "ovls";

        static void CMD_Help();
        static void CMD_Register();
        static void CMD_ReApply();
        static void CMD_Update();
        static void CMD_Erase();
        static void CMD_Clear();
        static void CMD_Flip();
        static void CMD_Reset3D();

        public:

        ~ConsoleManager() = default;

        [[nodiscard]] static inline ConsoleManager& GetSingleton() {
            static ConsoleManager Instance;
            return Instance;
        }

        static void Init() {
            logger::info("Loading Default Command List");
            RegisterCommand("help", CMD_Help, "Show this list");
            RegisterCommand("register", CMD_Register, "Register the current crosshair actor");
            RegisterCommand("apply", CMD_ReApply, "Apply saved ovl data");
            RegisterCommand("update", CMD_Update, "Update saved ovl data with current");
            RegisterCommand("erase", CMD_Erase, "Remove actor from cosave");
            RegisterCommand("flip", CMD_Flip, "Invert the order of overlays and reapply them");
            RegisterCommand("clear", CMD_Clear, "Clear overlays from actor but keep storage");
            RegisterCommand("reset3d", CMD_Reset3D, "Reset3D");

        }

        static void InstallHook();
        static void RegisterCommand(std::string_view a_cmdName, const std::function<void()>& a_callback, const std::string& a_desc);
        static bool Process(const std::string& a_msg);

        private:
        std::map<std::string, Command> RegisteredCommands;
    };


}