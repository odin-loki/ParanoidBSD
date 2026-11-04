module;

export module pbsd.userland.resizewin;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/resizewin/resizewin.c
export namespace pbsd::userland::usr_bin::resizewin {

[[nodiscard]] inline bool resizewin_list(char c) noexcept { return c == 'l'; }

} // namespace pbsd::userland::usr_bin::resizewin
