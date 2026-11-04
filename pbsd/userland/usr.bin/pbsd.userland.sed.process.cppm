module;

export module pbsd.userland.sed.process;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sed/process.c
export namespace pbsd::userland::usr_bin::sed::process {

[[nodiscard]] inline bool process_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sed::process
