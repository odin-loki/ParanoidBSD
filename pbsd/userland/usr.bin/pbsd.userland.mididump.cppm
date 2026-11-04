module;

export module pbsd.userland.mididump;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mididump/mididump.c
export namespace pbsd::userland::usr_bin::mididump {

[[nodiscard]] inline bool mididump_hex(char flag) noexcept { return flag == 'x'; }

} // namespace pbsd::userland::usr_bin::mididump
