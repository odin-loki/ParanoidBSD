module;

export module pbsd.userland.pax.cpio;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/cpio.c
export namespace pbsd::userland::bin::pax::cpio {

[[nodiscard]] inline bool cpio_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::cpio
