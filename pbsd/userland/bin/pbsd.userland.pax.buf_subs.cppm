module;

export module pbsd.userland.pax.buf_subs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/buf_subs.c
export namespace pbsd::userland::bin::pax::buf_subs {

[[nodiscard]] inline bool buf_subs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::buf_subs
