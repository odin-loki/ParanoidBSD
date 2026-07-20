module;

export module pbsd.userland.calendar.ostern;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/calendar/ostern.c
export namespace pbsd::userland::usr_bin::calendar::ostern {

[[nodiscard]] inline bool ostern_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::calendar::ostern
