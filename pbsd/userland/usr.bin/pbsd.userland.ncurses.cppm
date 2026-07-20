module;

export module pbsd.userland.ncurses;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ncurses/ncurses.c
export namespace pbsd::userland::usr_bin::ncurses {

[[nodiscard]] inline bool ncurses_flag(char c) noexcept { return c == 'T'; }

} // namespace pbsd::userland::usr_bin::ncurses
