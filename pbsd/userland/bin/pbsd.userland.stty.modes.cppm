module;

export module pbsd.userland.stty.modes;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/stty/modes.c
export namespace pbsd::userland::bin::stty::modes {

[[nodiscard]] inline bool modes_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::stty::modes
