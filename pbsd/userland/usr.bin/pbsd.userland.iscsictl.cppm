module;

export module pbsd.userland.iscsictl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/iscsictl/iscsictl.c
export namespace pbsd::userland::usr_bin::iscsictl {

[[nodiscard]] inline bool iscsictl_list(char c) noexcept { return c == 'l'; }

} // namespace pbsd::userland::usr_bin::iscsictl
