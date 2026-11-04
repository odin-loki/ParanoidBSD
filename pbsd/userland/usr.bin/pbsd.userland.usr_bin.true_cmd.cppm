module;

export module pbsd.userland.usr_bin.true_cmd;

export import pbsd.core;
export import pbsd.userland.true_cmd;

/// Port of hbsd/src/usr.bin/true/true.c — same semantics as bin/true(1).
export namespace pbsd::userland::usr_bin::true_cmd {

using pbsd::userland::bin::true_::run;
using pbsd::userland::bin::true_::exit_code;

} // namespace pbsd::userland::usr_bin::true_cmd
