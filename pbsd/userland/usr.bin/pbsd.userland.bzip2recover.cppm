module;

export module pbsd.userland.bzip2recover;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/contrib/bzip2/bzip2recover.c
export namespace pbsd::userland::usr_bin::bzip2recover {

[[nodiscard]] inline bool bzip2recover_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::bzip2recover
