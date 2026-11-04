module;

export module pbsd.userland.pax.ar_io;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/ar_io.c
export namespace pbsd::userland::bin::pax::ar_io {

[[nodiscard]] inline bool ar_io_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::ar_io
