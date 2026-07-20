module;

export module pbsd.userland.calendar.paskha;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/calendar/paskha.c
export namespace pbsd::userland::usr_bin::calendar::paskha {

[[nodiscard]] inline bool paskha_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::calendar::paskha
