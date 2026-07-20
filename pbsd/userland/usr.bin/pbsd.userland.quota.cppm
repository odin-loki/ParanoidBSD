module;

export module pbsd.userland.quota;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/quota/quota.c
export namespace pbsd::userland::usr_bin::quota {

[[nodiscard]] inline bool quota_verbose(char flag) noexcept { return flag == 'v'; }

} // namespace pbsd::userland::usr_bin::quota
