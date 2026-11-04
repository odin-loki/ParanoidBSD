module;

export module pbsd.userland.rpcgen.rpc_clntout;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_clntout.c
export namespace pbsd::userland::usr_bin::rpcgen::rpc_clntout {

[[nodiscard]] inline bool rpc_clntout_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::rpcgen::rpc_clntout
