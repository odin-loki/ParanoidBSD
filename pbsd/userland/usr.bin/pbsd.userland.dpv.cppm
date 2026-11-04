module;

export module pbsd.userland.dpv;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/dpv/dpv.c
export namespace pbsd::userland::usr_bin::dpv {

[[nodiscard]] inline bool dpv_quiet(char flag) noexcept { return flag == 'q'; }

} // namespace pbsd::userland::usr_bin::dpv
