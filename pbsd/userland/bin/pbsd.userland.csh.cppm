module;

export module pbsd.userland.csh;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/csh/iconv_stub.c
export namespace pbsd::userland::bin::csh {

[[nodiscard]] inline bool csh_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::bin::csh
