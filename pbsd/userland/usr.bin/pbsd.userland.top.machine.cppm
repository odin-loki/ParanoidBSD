module;

export module pbsd.userland.top.machine;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/top/machine.c
export namespace pbsd::userland::usr_bin::top::machine {

[[nodiscard]] inline bool machine_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::top::machine
