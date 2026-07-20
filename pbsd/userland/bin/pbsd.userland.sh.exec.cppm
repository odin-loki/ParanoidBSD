module;

export module pbsd.userland.sh.exec;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/exec.c
export namespace pbsd::userland::bin::sh::exec {

[[nodiscard]] inline bool exec_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::exec
