module;

export module pbsd.userland.dd.position;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/dd/position.c
export namespace pbsd::userland::bin::dd::position {

[[nodiscard]] inline bool position_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::dd::position
