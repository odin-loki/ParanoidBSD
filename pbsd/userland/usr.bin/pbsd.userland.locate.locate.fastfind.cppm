module;

export module pbsd.userland.locate.locate.fastfind;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/locate/locate/fastfind.c
export namespace pbsd::userland::usr_bin::locate::locate::fastfind {

[[nodiscard]] inline bool locate_fastfind_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::locate::locate::fastfind
