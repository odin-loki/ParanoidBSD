module;

export module pbsd.userland.pax.sel_subs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/sel_subs.c
export namespace pbsd::userland::bin::pax::sel_subs {

[[nodiscard]] inline bool sel_subs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::sel_subs
