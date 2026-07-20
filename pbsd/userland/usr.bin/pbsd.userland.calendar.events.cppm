module;

export module pbsd.userland.calendar.events;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/calendar/events.c
export namespace pbsd::userland::usr_bin::calendar::events {

[[nodiscard]] inline bool events_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::calendar::events
