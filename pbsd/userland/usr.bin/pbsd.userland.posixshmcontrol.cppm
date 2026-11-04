module;

export module pbsd.userland.posixshmcontrol;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/posixshmcontrol/posixshmcontrol.c
export namespace pbsd::userland::usr_bin::posixshmcontrol {

[[nodiscard]] inline bool posixshmcontrol_list(char c) noexcept { return c == 'l'; }

} // namespace pbsd::userland::usr_bin::posixshmcontrol
