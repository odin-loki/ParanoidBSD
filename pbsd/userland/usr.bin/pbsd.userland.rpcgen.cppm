module;

export module pbsd.userland.rpcgen;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_clntout.c
export namespace pbsd::userland::usr_bin::rpcgen {

[[nodiscard]] inline bool rpcgen_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::rpcgen
