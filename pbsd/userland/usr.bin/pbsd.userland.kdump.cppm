module;

export module pbsd.userland.kdump;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/kdump/kdump.c
export namespace pbsd::userland::usr_bin::kdump {

[[nodiscard]] inline bool kdump_follow(char flag) noexcept { return flag == 'f'; }

} // namespace pbsd::userland::usr_bin::kdump
