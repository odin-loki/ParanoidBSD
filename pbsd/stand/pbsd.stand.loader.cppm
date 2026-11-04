module;
#include <cstdint>

export module pbsd.stand.loader;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/common/commands.c, boot.c — loader command opcodes.
export namespace pbsd::stand::loader {

enum class Cmd : unsigned int {
    Boot     = 0,
    Load     = 1,
    Unload   = 2,
    Ls       = 3,
    Cat      = 4,
    More     = 5,
    Include  = 6,
    Help     = 7,
    Reboot   = 8,
    Pwd      = 9,
    Set      = 10,
    Echo     = 11,
    Read     = 12,
    Bcopy    = 13,
    Malloc   = 14,
    Free     = 15,
};

struct CmdEntry {
    Cmd         cmd{};
    const char* name{};
    bool        needs_arg{};
};

inline constexpr CmdEntry kCmdTable[] = {
    {Cmd::Boot, "boot", false},
    {Cmd::Load, "load", true},
    {Cmd::Unload, "unload", true},
    {Cmd::Ls, "ls", true},
    {Cmd::Cat, "cat", true},
    {Cmd::Help, "help", false},
    {Cmd::Reboot, "reboot", false},
    {Cmd::Set, "set", true},
    {Cmd::Echo, "echo", true},
};

[[nodiscard]] inline constexpr std::size_t cmd_table_size() noexcept {
    return sizeof(kCmdTable) / sizeof(kCmdTable[0]);
}

[[nodiscard]] inline Status validate_cmd(Cmd c) noexcept {
    if (static_cast<unsigned>(c) > static_cast<unsigned>(Cmd::Free)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::loader
