module;

export module pbsd.userland.find.operator_;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/find/operator.c
export namespace pbsd::userland::usr_bin::find::operator_ {

[[nodiscard]] inline bool operator__flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::find::operator_
