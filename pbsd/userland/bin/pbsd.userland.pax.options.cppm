module;

export module pbsd.userland.pax.options;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/options.c
export namespace pbsd::userland::bin::pax::options {

[[nodiscard]] inline bool options_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::options
