module;

export module pbsd.userland.mkimg.uuid;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/uuid.c
export namespace pbsd::userland::usr_bin::mkimg::uuid {

[[nodiscard]] inline bool uuid_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::uuid
