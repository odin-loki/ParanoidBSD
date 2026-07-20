module;

export module pbsd.userland.calendar.pom;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/calendar/pom.c
export namespace pbsd::userland::usr_bin::calendar::pom {

[[nodiscard]] inline bool pom_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::calendar::pom
