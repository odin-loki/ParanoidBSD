module;

export module pbsd.userland.c89;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/c89/c89.c
export namespace pbsd::userland::usr_bin::c89 {

[[nodiscard]] inline bool c89_pedantic(char flag) noexcept { return flag == 'p'; }

} // namespace pbsd::userland::usr_bin::c89
