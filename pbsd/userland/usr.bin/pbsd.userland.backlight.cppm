module;

export module pbsd.userland.backlight;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/backlight/backlight.c
export namespace pbsd::userland::usr_bin::backlight {

[[nodiscard]] inline bool backlight_on(char flag) noexcept { return flag == 'n'; }

} // namespace pbsd::userland::usr_bin::backlight
