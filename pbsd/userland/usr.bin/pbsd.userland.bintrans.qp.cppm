module;

export module pbsd.userland.bintrans.qp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bintrans/qp.c
export namespace pbsd::userland::usr_bin::bintrans::qp {

[[nodiscard]] inline bool qp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::bintrans::qp
