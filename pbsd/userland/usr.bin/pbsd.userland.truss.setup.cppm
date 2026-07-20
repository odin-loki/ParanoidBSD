module;

export module pbsd.userland.truss.setup;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/truss/setup.c
export namespace pbsd::userland::usr_bin::truss::setup {

[[nodiscard]] inline bool setup_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::truss::setup
