module;

export module pbsd.userland.bintrans;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bintrans/bintrans.c
export namespace pbsd::userland::usr_bin::bintrans {

[[nodiscard]] inline bool bintrans_verbose(char flag) noexcept { return flag == 'v'; }

} // namespace pbsd::userland::usr_bin::bintrans
