module;

export module pbsd.userland.patch.inp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/patch/inp.c
export namespace pbsd::userland::usr_bin::patch::inp {

[[nodiscard]] inline bool inp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::patch::inp
