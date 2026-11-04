module;

export module pbsd.userland.rpcgen.rpc_tblout;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_tblout.c
export namespace pbsd::userland::usr_bin::rpcgen::rpc_tblout {

[[nodiscard]] inline bool rpc_tblout_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::rpcgen::rpc_tblout
