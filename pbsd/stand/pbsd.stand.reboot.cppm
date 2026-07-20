module;
#include <cstdint>

export module pbsd.stand.reboot;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/reboot.h — RB_* reboot flags.
export namespace pbsd::stand::reboot {

enum class Flag : unsigned int {
    AutoBoot   = 0x0001,
    AskName    = 0x0002,
    Single     = 0x0004,
    Dump       = 0x0100,
    InitName   = 0x0200,
    PowerOff   = 0x0400,
    PowerCycle = 0x0800,
    BootInfo   = 0x1000,
    Kdb        = 0x2000,
    Color      = 0x4000,
    Pause      = 0x8000,
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::PowerOff))
        && (flags & static_cast<unsigned>(Flag::PowerCycle))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool wants_dump(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(Flag::Dump)) != 0;
}

} // namespace pbsd::stand::reboot
