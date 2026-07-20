module;

export module pbsd.userland.rctl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rctl/rctl.c
export namespace pbsd::userland::usr_bin::rctl {

[[nodiscard]] inline bool rctl_list_rules(char flag) noexcept { return flag == 'l'; }

} // namespace pbsd::userland::usr_bin::rctl
