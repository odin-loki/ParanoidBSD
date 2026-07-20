module;

export module pbsd.userland.tip.tip.remote;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/remote.c
export namespace pbsd::userland::usr_bin::tip::tip::remote {

[[nodiscard]] inline bool tip_remote_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::remote
