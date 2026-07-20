module;

export module pbsd.userland.bmake;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/contrib/bmake/main.c
export namespace pbsd::userland::usr_bin::bmake {

[[nodiscard]] inline bool bmake_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::bmake
