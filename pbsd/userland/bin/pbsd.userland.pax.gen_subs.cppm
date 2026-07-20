module;

export module pbsd.userland.pax.gen_subs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/gen_subs.c
export namespace pbsd::userland::bin::pax::gen_subs {

[[nodiscard]] inline bool gen_subs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::gen_subs
