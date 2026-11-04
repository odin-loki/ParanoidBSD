module;

export module pbsd.userland.locate.locate.locate;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/locate/locate/locate.c
export namespace pbsd::userland::usr_bin::locate::locate::locate {

[[nodiscard]] inline bool locate_locate_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::locate::locate::locate
