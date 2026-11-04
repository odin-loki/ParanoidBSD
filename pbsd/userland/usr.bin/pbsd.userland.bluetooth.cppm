module;

export module pbsd.userland.bluetooth;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bluetooth/bthost/bthost.c
export namespace pbsd::userland::usr_bin::bluetooth {

[[nodiscard]] inline bool bluetooth_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::bluetooth
