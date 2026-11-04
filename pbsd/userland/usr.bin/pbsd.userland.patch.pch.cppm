module;

export module pbsd.userland.patch.pch;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/patch/pch.c
export namespace pbsd::userland::usr_bin::patch::pch {

[[nodiscard]] inline bool pch_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::patch::pch
