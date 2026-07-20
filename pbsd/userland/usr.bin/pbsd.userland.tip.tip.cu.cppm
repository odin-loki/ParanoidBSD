module;

export module pbsd.userland.tip.tip.cu;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/cu.c
export namespace pbsd::userland::usr_bin::tip::tip::cu {

[[nodiscard]] inline bool tip_cu_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::cu
