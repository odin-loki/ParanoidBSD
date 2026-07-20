module;

export module pbsd.userland.compile_et;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/compile_et/compile_et.c
export namespace pbsd::userland::usr_bin::compile_et {

[[nodiscard]] inline bool compile_et_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::compile_et
