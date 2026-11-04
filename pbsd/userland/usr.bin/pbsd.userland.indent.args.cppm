module;

export module pbsd.userland.indent.args;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/indent/args.c
export namespace pbsd::userland::usr_bin::indent::args {

[[nodiscard]] inline bool args_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::indent::args
