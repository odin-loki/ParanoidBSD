module;

export module pbsd.userland.ar.write;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ar/write.c
export namespace pbsd::userland::usr_bin::ar::write {

[[nodiscard]] inline bool write_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ar::write
