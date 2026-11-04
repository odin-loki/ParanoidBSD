module;

export module pbsd.userland.etdump;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/etdump/etdump.c
export namespace pbsd::userland::usr_bin::etdump {

[[nodiscard]] inline bool etdump_all(char flag) noexcept { return flag == 'a'; }

} // namespace pbsd::userland::usr_bin::etdump
