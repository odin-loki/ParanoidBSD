module;

export module pbsd.userland.rpcgen.rpc_cout;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_cout.c
export namespace pbsd::userland::usr_bin::rpcgen::rpc_cout {

[[nodiscard]] inline bool rpc_cout_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::rpcgen::rpc_cout
