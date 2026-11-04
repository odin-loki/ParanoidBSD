module;

export module pbsd.userland.ktrdump;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ktrdump/ktrdump.c
export namespace pbsd::userland::usr_bin::ktrdump {

[[nodiscard]] inline bool ktrdump_follow(char c) noexcept { return c == 'f'; }

} // namespace pbsd::userland::usr_bin::ktrdump
