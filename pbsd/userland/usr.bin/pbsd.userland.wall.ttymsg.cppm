module;

export module pbsd.userland.wall.ttymsg;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/wall/ttymsg.c
export namespace pbsd::userland::usr_bin::wall::ttymsg {

[[nodiscard]] inline bool ttymsg_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::wall::ttymsg
