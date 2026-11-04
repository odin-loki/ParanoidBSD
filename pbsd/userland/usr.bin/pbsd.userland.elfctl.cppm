module;

export module pbsd.userland.elfctl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/elfctl/elfctl.c
export namespace pbsd::userland::usr_bin::elfctl {

[[nodiscard]] inline bool elfctl_list(char flag) noexcept { return flag == 'l'; }

} // namespace pbsd::userland::usr_bin::elfctl
