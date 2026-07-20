module;

export module pbsd.userland.xopo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/xopo/xopo.c
export namespace pbsd::userland::usr_bin::xopo {

[[nodiscard]] inline bool xopo_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::xopo
