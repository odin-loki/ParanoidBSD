module;

export module pbsd.userland.usbhidaction;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/usbhidaction/usbhidaction.c
export namespace pbsd::userland::usr_bin::usbhidaction {

[[nodiscard]] inline bool usbhidaction_list(char c) noexcept { return c == 'l'; }

} // namespace pbsd::userland::usr_bin::usbhidaction
