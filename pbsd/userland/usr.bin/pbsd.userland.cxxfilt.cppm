module;

export module pbsd.userland.cxxfilt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cxxfilt/cxxfilt.c
export namespace pbsd::userland::usr_bin::cxxfilt {

[[nodiscard]] inline bool cxxfilt_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::cxxfilt
