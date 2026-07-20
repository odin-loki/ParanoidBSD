module;

export module pbsd.userland.pax.tty_subs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/tty_subs.c
export namespace pbsd::userland::bin::pax::tty_subs {

[[nodiscard]] inline bool tty_subs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::tty_subs
