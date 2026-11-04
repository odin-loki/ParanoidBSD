module;
#include <cstdint>

export module pbsd.stand.common;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/common/boot.c, commands.c, load_elf.c
export namespace pbsd::stand::common {

inline constexpr unsigned kBootFVerbose   = 0x0001;
inline constexpr unsigned kBootFQuiet     = 0x0002;
inline constexpr unsigned kBootFAskName   = 0x0004;
inline constexpr unsigned kBootFSingle    = 0x0008;
inline constexpr unsigned kBootFDual       = 0x0010;
inline constexpr unsigned kBootFNoSwap     = 0x0020;
inline constexpr unsigned kBootFRootFromLabel = 0x0040;
inline constexpr unsigned kBootFRootFromUuid  = 0x0080;
inline constexpr unsigned kBootFRootFromZfs   = 0x0100;

enum class LoaderCmd : unsigned int {
    Boot     = 0,
    Load     = 1,
    Unload   = 2,
    Ls       = 3,
    More     = 4,
    Set      = 5,
    Show     = 6,
    Help     = 7,
    Reboot   = 8,
    Include  = 9,
    Read     = 10,
    BcacheCstat = 11,
};

enum class ElfClass : unsigned char {
    None = 0,
    Elf32 = 1,
    Elf64 = 2,
};

enum class ElfMachine : unsigned short {
    None   = 0,
    I386   = 3,
    Amd64  = 62,
    Arm    = 40,
    Aarch64 = 183,
};

struct CmdEntry {
    LoaderCmd   cmd{};
    const char* name{};
};

inline constexpr CmdEntry kLoaderCmdTable[] = {
    {LoaderCmd::Boot, "boot"},
    {LoaderCmd::Load, "load"},
    {LoaderCmd::Unload, "unload"},
    {LoaderCmd::Ls, "ls"},
    {LoaderCmd::Set, "set"},
    {LoaderCmd::Show, "show"},
    {LoaderCmd::Help, "help"},
    {LoaderCmd::Reboot, "reboot"},
    {LoaderCmd::Include, "include"},
    {LoaderCmd::Read, "read"},
};

[[nodiscard]] inline constexpr std::size_t loader_cmd_table_size() noexcept {
    return sizeof(kLoaderCmdTable) / sizeof(kLoaderCmdTable[0]);
}

[[nodiscard]] inline Status validate_boot_flags(unsigned flags) noexcept {
    if ((flags & kBootFVerbose) != 0 && (flags & kBootFQuiet) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool root_from_zfs(unsigned flags) noexcept {
    return (flags & kBootFRootFromZfs) != 0;
}

[[nodiscard]] inline Status validate_elf_machine(ElfClass cls, ElfMachine mach) noexcept {
    if (cls == ElfClass::Elf64 && mach != ElfMachine::Amd64 && mach != ElfMachine::Aarch64) {
        return Status::Protocol;
    }
    if (cls == ElfClass::Elf32 && mach != ElfMachine::I386 && mach != ElfMachine::Arm) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::common
