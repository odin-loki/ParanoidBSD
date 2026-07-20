module;

export module pbsd.userland.usbhidctl.usbhid;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/usbhidctl/usbhid.c
export namespace pbsd::userland::usr_bin::usbhidctl::usbhid {

[[nodiscard]] inline bool usbhid_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::usbhidctl::usbhid
