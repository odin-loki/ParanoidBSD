module;

export module pbsd.userland.vtfontcvt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/vtfontcvt/vtfontcvt.c
export namespace pbsd::userland::usr_bin::vtfontcvt {

[[nodiscard]] inline bool vtfontcvt_verbose(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::vtfontcvt
