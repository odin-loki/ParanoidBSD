module;

export module pbsd.userland.top.screen;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/top/screen.c
export namespace pbsd::userland::usr_bin::top::screen {

[[nodiscard]] inline bool screen_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::top::screen
