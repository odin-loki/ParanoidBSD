module;

export module pbsd.userland.fortune.unstr.unstr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/fortune/unstr/unstr.c
export namespace pbsd::userland::usr_bin::fortune::unstr::unstr {

[[nodiscard]] inline bool unstr_unstr_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::fortune::unstr::unstr
