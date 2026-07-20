module;

export module pbsd.userland.sed.compile;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sed/compile.c
export namespace pbsd::userland::usr_bin::sed::compile {

[[nodiscard]] inline bool compile_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sed::compile
