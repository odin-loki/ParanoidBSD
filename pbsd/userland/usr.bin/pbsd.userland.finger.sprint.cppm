module;

export module pbsd.userland.finger.sprint;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/finger/sprint.c
export namespace pbsd::userland::usr_bin::finger::sprint {

[[nodiscard]] inline bool sprint_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::finger::sprint
