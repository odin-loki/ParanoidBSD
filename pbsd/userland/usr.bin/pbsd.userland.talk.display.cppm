module;

export module pbsd.userland.talk.display;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/display.c
export namespace pbsd::userland::usr_bin::talk::display {

[[nodiscard]] inline bool display_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::display
