module;

export module pbsd.userland.calendar.sunpos;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/calendar/sunpos.c
export namespace pbsd::userland::usr_bin::calendar::sunpos {

[[nodiscard]] inline bool sunpos_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::calendar::sunpos
