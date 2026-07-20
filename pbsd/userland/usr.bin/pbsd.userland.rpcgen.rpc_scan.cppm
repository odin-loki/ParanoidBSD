module;

export module pbsd.userland.rpcgen.rpc_scan;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_scan.c
export namespace pbsd::userland::usr_bin::rpcgen::rpc_scan {

[[nodiscard]] inline bool rpc_scan_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::rpcgen::rpc_scan
