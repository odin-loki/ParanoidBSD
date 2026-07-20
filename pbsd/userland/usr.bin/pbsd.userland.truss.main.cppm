module;

export module pbsd.userland.truss.main;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/truss/main.c
export namespace pbsd::userland::usr_bin::truss::main {

[[nodiscard]] inline bool main_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::truss::main
