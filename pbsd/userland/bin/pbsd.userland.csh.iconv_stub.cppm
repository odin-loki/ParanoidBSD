module;

export module pbsd.userland.csh.iconv_stub;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/csh/iconv_stub.c
export namespace pbsd::userland::bin::csh::iconv_stub {

[[nodiscard]] inline bool iconv_stub_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::csh::iconv_stub
