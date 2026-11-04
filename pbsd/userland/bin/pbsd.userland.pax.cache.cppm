module;

export module pbsd.userland.pax.cache;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/cache.c
export namespace pbsd::userland::bin::pax::cache {

[[nodiscard]] inline bool cache_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::cache
