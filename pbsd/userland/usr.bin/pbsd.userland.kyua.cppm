module;

export module pbsd.userland.kyua;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/kyua/kyua.c
export namespace pbsd::userland::usr_bin::kyua {

[[nodiscard]] inline bool kyua_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::kyua
