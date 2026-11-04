module;

export module pbsd.userland.bsddialog;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bsddialog/bsddialog.c
export namespace pbsd::userland::usr_bin::bsddialog {

[[nodiscard]] inline bool bsddialog_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::bsddialog
