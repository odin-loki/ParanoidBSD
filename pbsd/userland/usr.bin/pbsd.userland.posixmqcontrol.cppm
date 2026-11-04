module;

export module pbsd.userland.posixmqcontrol;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/posixmqcontrol/posixmqcontrol.c
export namespace pbsd::userland::usr_bin::posixmqcontrol {

[[nodiscard]] inline bool posixmqcontrol_list(char c) noexcept { return c == 'l'; }

} // namespace pbsd::userland::usr_bin::posixmqcontrol
