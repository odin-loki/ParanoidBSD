module;

export module pbsd.userland.pax.file_subs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/file_subs.c
export namespace pbsd::userland::bin::pax::file_subs {

[[nodiscard]] inline bool file_subs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::file_subs
