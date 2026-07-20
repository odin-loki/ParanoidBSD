module;

export module pbsd.userland.rpcgen.rpc_util;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_util.c
export namespace pbsd::userland::usr_bin::rpcgen::rpc_util {

[[nodiscard]] inline bool rpc_util_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::rpcgen::rpc_util
