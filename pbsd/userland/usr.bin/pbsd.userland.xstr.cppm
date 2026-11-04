module;

export module pbsd.userland.xstr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/xstr/xstr.c
export namespace pbsd::userland::usr_bin::xstr {

[[nodiscard]] inline bool xstr_quote(char c) noexcept { return c == 'q'; }

} // namespace pbsd::userland::usr_bin::xstr
