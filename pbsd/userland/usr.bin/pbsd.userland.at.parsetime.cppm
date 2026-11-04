module;

export module pbsd.userland.at.parsetime;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/at/parsetime.c
export namespace pbsd::userland::usr_bin::at::parsetime {

[[nodiscard]] inline bool parsetime_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::at::parsetime
