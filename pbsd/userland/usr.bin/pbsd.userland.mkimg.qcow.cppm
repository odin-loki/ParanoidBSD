module;

export module pbsd.userland.mkimg.qcow;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/qcow.c
export namespace pbsd::userland::usr_bin::mkimg::qcow {

[[nodiscard]] inline bool qcow_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::qcow
