module;

export module pbsd.userland.ed.main;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ed/main.c
export namespace pbsd::userland::bin::ed::main {

[[nodiscard]] inline bool main_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ed::main
