module;

export module pbsd.userland.bluetooth.bthost.bthost;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bluetooth/bthost/bthost.c
export namespace pbsd::userland::usr_bin::bluetooth::bthost::bthost {

[[nodiscard]] inline bool bthost_bthost_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::bluetooth::bthost::bthost
