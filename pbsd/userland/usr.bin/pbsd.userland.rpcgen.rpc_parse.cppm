module;

export module pbsd.userland.rpcgen.rpc_parse;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_parse.c
export namespace pbsd::userland::usr_bin::rpcgen::rpc_parse {

[[nodiscard]] inline bool rpc_parse_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::rpcgen::rpc_parse
