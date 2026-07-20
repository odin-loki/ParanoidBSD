module;

export module pbsd.userland.rpcinfo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcinfo/rpcinfo.c
export namespace pbsd::userland::usr_bin::rpcinfo {

[[nodiscard]] inline bool rpcinfo_list_all(char flag) noexcept { return flag == 'p'; }

} // namespace pbsd::userland::usr_bin::rpcinfo
