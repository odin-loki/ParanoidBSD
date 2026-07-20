module;

export module pbsd.userland.tip.tip.hunt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/hunt.c
export namespace pbsd::userland::usr_bin::tip::tip::hunt {

[[nodiscard]] inline bool tip_hunt_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::hunt
