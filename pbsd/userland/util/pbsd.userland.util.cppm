export module pbsd.userland.util;

export import pbsd.userland.util.numbers;
export import pbsd.userland.util.format;
export import pbsd.userland.util.host;
export import pbsd.userland.util.property;
export import pbsd.userland.util.cpuset;
export import pbsd.userland.util.flopen;
export import pbsd.userland.util.pidfile;
export import pbsd.userland.util.filemode;
export import pbsd.userland.util.fparseln;
export import pbsd.userland.util.secure;
export import pbsd.userland.util.paths;
export import pbsd.userland.util.mntopts;
export import pbsd.userland.util.uucplock;
export import pbsd.userland.util.login_times;
export import pbsd.userland.util.expand_number;
export import pbsd.userland.util.ftime;
export import pbsd.userland.util.getlocalbase;
export import pbsd.userland.util.gr_util;
export import pbsd.userland.util.hexdump;
export import pbsd.userland.util.kinfo_getproc;
export import pbsd.userland.util.kld;
export import pbsd.userland.util.login_cap;
export import pbsd.userland.util.pty;
export import pbsd.userland.util.pw_util;
export import pbsd.userland.util.trimdomain;
export import pbsd.userland.util.secure_path;

export import pbsd.handles;
import pbsd.core;

/// libutil umbrella — re-exports partition modules + legacy handle aliases.
export namespace pbsd::userland::util {

using PropertyBag = PropertyList;

} // namespace pbsd::userland::util
