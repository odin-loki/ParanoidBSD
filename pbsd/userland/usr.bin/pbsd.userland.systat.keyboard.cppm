module;

export module pbsd.userland.systat.keyboard;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/keyboard.c
export namespace pbsd::userland::usr_bin::systat::keyboard {

[[nodiscard]] inline bool keyboard_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::keyboard
