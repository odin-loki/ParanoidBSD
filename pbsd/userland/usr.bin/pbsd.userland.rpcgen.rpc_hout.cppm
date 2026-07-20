module;

export module pbsd.userland.rpcgen.rpc_hout;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_hout.c
export namespace pbsd::userland::usr_bin::rpcgen::rpc_hout {

[[nodiscard]] inline bool rpc_hout_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::rpcgen::rpc_hout
