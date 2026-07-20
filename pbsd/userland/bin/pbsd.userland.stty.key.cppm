module;

export module pbsd.userland.stty.key;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/stty/key.c
export namespace pbsd::userland::bin::stty::key {

[[nodiscard]] inline bool key_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::stty::key
