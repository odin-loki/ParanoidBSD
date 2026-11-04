module;

export module pbsd.userland.proccontrol;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/proccontrol/proccontrol.c
export namespace pbsd::userland::usr_bin::proccontrol {

[[nodiscard]] inline bool proccontrol_list(char c) noexcept { return c == 'l'; }

} // namespace pbsd::userland::usr_bin::proccontrol
