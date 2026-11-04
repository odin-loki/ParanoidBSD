module;

export module pbsd.userland.top.display;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/top/display.c
export namespace pbsd::userland::usr_bin::top::display {

[[nodiscard]] inline bool display_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::top::display
