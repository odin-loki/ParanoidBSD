module;

export module pbsd.userland.calendar.day;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/calendar/day.c
export namespace pbsd::userland::usr_bin::calendar::day {

[[nodiscard]] inline bool day_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::calendar::day
