module;

export module pbsd.userland.calendar.dates;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/calendar/dates.c
export namespace pbsd::userland::usr_bin::calendar::dates {

[[nodiscard]] inline bool dates_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::calendar::dates
