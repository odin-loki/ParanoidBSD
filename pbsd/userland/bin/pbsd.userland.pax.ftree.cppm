module;

export module pbsd.userland.pax.ftree;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/ftree.c
export namespace pbsd::userland::bin::pax::ftree {

[[nodiscard]] inline bool ftree_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::ftree
