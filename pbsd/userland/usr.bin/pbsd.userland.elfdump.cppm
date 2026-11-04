module;

export module pbsd.userland.elfdump;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/elfdump/elfdump.c
export namespace pbsd::userland::usr_bin::elfdump {

[[nodiscard]] inline bool elfdump_all(char flag) noexcept { return flag == 'a'; }

} // namespace pbsd::userland::usr_bin::elfdump
