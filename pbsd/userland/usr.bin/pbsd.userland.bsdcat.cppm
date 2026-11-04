module;

export module pbsd.userland.bsdcat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/contrib/libarchive/cat/bsdcat.c
export namespace pbsd::userland::usr_bin::bsdcat {

[[nodiscard]] inline bool bsdcat_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::bsdcat
