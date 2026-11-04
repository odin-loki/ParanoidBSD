module;

export module pbsd.userland.clang;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/clang.c
export namespace pbsd::userland::usr_bin::clang {

[[nodiscard]] inline bool clang_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::clang
