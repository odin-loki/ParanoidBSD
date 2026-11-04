module;

export module pbsd.userland.calendar.locale;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/calendar/locale.c
export namespace pbsd::userland::usr_bin::calendar::locale {

[[nodiscard]] inline bool locale_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::calendar::locale
