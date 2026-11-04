module;

export module pbsd.userland.rwho;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rwho/rwho.c
export namespace pbsd::userland::usr_bin::rwho {

[[nodiscard]] inline bool rwho_all_hosts(char flag) noexcept { return flag == 'a'; }

} // namespace pbsd::userland::usr_bin::rwho
