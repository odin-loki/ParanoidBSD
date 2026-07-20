module;

export module pbsd.userland.pax.tar;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/tar.c
export namespace pbsd::userland::bin::pax::tar {

[[nodiscard]] inline bool tar_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::tar
