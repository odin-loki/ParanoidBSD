module;

export module pbsd.userland.calendar.parsedata;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/calendar/parsedata.c
export namespace pbsd::userland::usr_bin::calendar::parsedata {

[[nodiscard]] inline bool parsedata_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::calendar::parsedata
