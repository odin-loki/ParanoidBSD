module;

export module pbsd.userland.pax.ar_subs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/ar_subs.c
export namespace pbsd::userland::bin::pax::ar_subs {

[[nodiscard]] inline bool ar_subs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::ar_subs
