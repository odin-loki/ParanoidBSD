module;

export module pbsd.userland.etdump.output_shell;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/etdump/output_shell.c
export namespace pbsd::userland::usr_bin::etdump::output_shell {

[[nodiscard]] inline bool output_shell_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::etdump::output_shell
