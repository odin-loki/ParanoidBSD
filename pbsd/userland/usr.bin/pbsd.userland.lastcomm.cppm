module;

export module pbsd.userland.lastcomm;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lastcomm/lastcomm.c
export namespace pbsd::userland::usr_bin::lastcomm {

[[nodiscard]] inline bool lastcomm_user_only(char flag) noexcept { return flag == 'S'; }

} // namespace pbsd::userland::usr_bin::lastcomm
