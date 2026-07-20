module;

export module pbsd.userland.mdo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mdo/mdo.c
export namespace pbsd::userland::usr_bin::mdo {

[[nodiscard]] inline bool mdo_list(char flag) noexcept { return flag == 'l'; }

} // namespace pbsd::userland::usr_bin::mdo
