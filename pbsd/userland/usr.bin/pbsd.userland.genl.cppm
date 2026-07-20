module;

export module pbsd.userland.genl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/genl/genl.c
export namespace pbsd::userland::usr_bin::genl {

[[nodiscard]] inline bool genl_list(char flag) noexcept { return flag == 'l'; }

} // namespace pbsd::userland::usr_bin::genl
