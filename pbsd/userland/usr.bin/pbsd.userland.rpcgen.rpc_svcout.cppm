module;

export module pbsd.userland.rpcgen.rpc_svcout;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_svcout.c
export namespace pbsd::userland::usr_bin::rpcgen::rpc_svcout {

[[nodiscard]] inline bool rpc_svcout_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::rpcgen::rpc_svcout
