module;

export module pbsd.userland.tip;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/tip.c
export namespace pbsd::userland::usr_bin::tip {

[[nodiscard]] inline bool tip_script(char c) noexcept { return c == 's'; }

} // namespace pbsd::userland::usr_bin::tip
