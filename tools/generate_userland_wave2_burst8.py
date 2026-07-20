#!/usr/bin/env python3
"""Generate Wave 2 burst 8 userland C++23 modules (38 tools)."""
from __future__ import annotations

import textwrap
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
USERLAND = ROOT / "pbsd" / "userland"

# (name, category_dir, ns_segment, hbsd_source, kind, extra)
# category_dir: usr.bin | sbin | usr.sbin
# ns_segment: usr_bin | sbin | usr_sbin
TOOLS: list[tuple[str, str, str, str, str, dict]] = [
    ("bzip2", "usr.bin", "usr_bin", "hbsd/src/usr.bin/bzip2", "flags", {"flags": "cdfkqvzt123456789", "magic": "BZh"}),
    ("xz", "usr.bin", "usr_bin", "hbsd/src/usr.bin/xz", "flags", {"flags": "cdfkqvzt0123456789", "magic": "\\xfd7zXZ"}),
    ("curl", "usr.bin", "usr_bin", "hbsd/src/contrib/curl", "stub", {"scheme": "http"}),
    ("scp", "usr.bin", "usr_bin", "hbsd/src/crypto/openssh/scp.c", "flags", {"flags": "12346BCpqrv"}),
    ("rsync", "usr.bin", "usr_bin", "hbsd/src/contrib/rsync", "stub", {}),
    ("compress", "usr.bin", "usr_bin", "hbsd/src/usr.bin/compress/compress.c", "flags", {"flags": "bfv", "magic": "\\x1f\\x9d"}),
    ("sysrc", "usr.sbin", "usr_sbin", "hbsd/src/usr.sbin/sysrc/sysrc", "subcmd", {
        "cmds": ["get", "set", "del", "exists", "list", "all"],
    }),
    ("service", "usr.sbin", "usr_sbin", "hbsd/src/usr.sbin/service/service.sh", "subcmd", {
        "cmds": ["start", "stop", "restart", "reload", "status", "enable", "disable"],
    }),
    ("services", "usr.sbin", "usr_sbin", "hbsd/src/usr.sbin/services_mkdb", "flags", {"flags": "o:q"}),
    ("bectl", "sbin", "sbin", "hbsd/src/sbin/bectl/bectl.c", "subcmd", {
        "cmds": ["activate", "check", "create", "destroy", "export", "import", "jail",
                 "list", "mount", "rename", "ujail", "unjail", "umount", "unmount"],
    }),
    ("camcontrol", "sbin", "sbin", "hbsd/src/sbin/camcontrol/camcontrol.c", "subcmd", {
        "cmds": ["tur", "inquiry", "devlist", "rescan", "start", "stop", "modepage", "tags"],
    }),
    ("geom", "sbin", "sbin", "hbsd/src/sbin/geom/core/geom.c", "subcmd", {
        "cmds": ["list", "status", "help", "load", "unload", "part", "label", "mirror", "stripe"],
    }),
    ("gpart", "sbin", "sbin", "hbsd/src/sbin/geom/core/geom.c", "subcmd", {
        "cmds": ["create", "add", "delete", "show", "backup", "restore", "resize", "bootcode"],
    }),
    ("mdconfig", "sbin", "sbin", "hbsd/src/sbin/mdconfig/mdconfig.c", "flags", {"flags": "adlru:f:s:S:u:L:xy:t:o:n"}),
    ("dumpon", "sbin", "sbin", "hbsd/src/sbin/dumpon/dumpon.c", "flags", {"flags": "v"}),
    ("savecore", "sbin", "sbin", "hbsd/src/sbin/savecore/savecore.c", "flags", {"flags": "fknvz"}),
    ("fsck", "sbin", "sbin", "hbsd/src/sbin/fsck/fsck.c", "flags", {"flags": "BCdvpfFnyl:t:T:c:"}),
    ("fsck_ffs", "sbin", "sbin", "hbsd/src/sbin/fsck_ffs", "flags", {"flags": "Bdfmnpfy"}),
    ("fsck_msdosfs", "sbin", "sbin", "hbsd/src/sbin/fsck_msdosfs", "flags", {"flags": "fnpvy"}),
    ("newfs", "sbin", "sbin", "hbsd/src/sbin/newfs/newfs.c", "flags", {"flags": "E:L:O:S:b:i:m:n:o:s:t:"}),
    ("newfs_msdos", "sbin", "sbin", "hbsd/src/sbin/newfs_msdos", "flags", {"flags": "F:I:O:S:a:b:c:f:i:k:m:n:o:s:v:"}),
    ("tunefs", "sbin", "sbin", "hbsd/src/sbin/tunefs/tunefs.c", "flags", {"flags": "A:a:e:j:l:m:n:o:p:"}),
    ("growfs", "sbin", "sbin", "hbsd/src/sbin/growfs/growfs.c", "flags", {"flags": "Ny:s:"}),
    ("zfs", "sbin", "sbin", "hbsd/src/sys/contrib/openzfs/cmd/zfs", "subcmd", {
        "cmds": ["create", "destroy", "list", "mount", "umount", "snapshot", "rollback",
                 "send", "receive", "set", "get", "rename", "clone", "promote"],
    }),
    ("zpool", "sbin", "sbin", "hbsd/src/sys/contrib/openzfs/cmd/zpool/zpool_main.c", "subcmd", {
        "cmds": ["create", "destroy", "list", "status", "import", "export", "add", "remove",
                 "clear", "scrub", "upgrade", "history", "iostat"],
    }),
    ("dmesg", "sbin", "sbin", "hbsd/src/sbin/dmesg", "flags", {"flags": "acTt"}),
    ("dumpfs", "sbin", "sbin", "hbsd/src/sbin/dumpfs", "flags", {"flags": "acijklmsu:"}),
    ("fdisk", "sbin", "sbin", "hbsd/src/sbin/fdisk", "flags", {"flags": "BIbfhiqsuvy:"}),
    ("bsdlabel", "sbin", "sbin", "hbsd/src/sbin/bsdlabel", "flags", {"flags": "BRWbemnwx:"}),
    ("swapon", "sbin", "sbin", "hbsd/src/sbin/swapon", "flags", {"flags": "a"}),
    ("shutdown", "sbin", "sbin", "hbsd/src/sbin/shutdown", "flags", {"flags": "hknr:o:p:"}),
    ("reboot", "sbin", "sbin", "hbsd/src/sbin/reboot", "flags", {"flags": "dnqrv"}),
    ("kldload", "sbin", "sbin", "hbsd/src/sbin/kldload", "flags", {"flags": "v"}),
    ("hexdump", "usr.bin", "usr_bin", "hbsd/src/usr.bin/hexdump", "flags", {"flags": "bcCde:f:n:os:v:x"}),
    ("stat", "usr.bin", "usr_bin", "hbsd/src/usr.bin/stat", "flags", {"flags": "FLn:qt:x:f:l:s:"}),
    ("cksum", "usr.bin", "usr_bin", "hbsd/src/usr.bin/cksum", "flags", {"flags": "no:s:"}),
    ("netstat", "usr.bin", "usr_bin", "hbsd/src/usr.bin/netstat", "flags", {"flags": "AaBbdgIilmnprsuWw:"}),
    ("sockstat", "usr.bin", "usr_bin", "hbsd/src/usr.bin/sockstat", "flags", {"flags": "46clpPq:"}),
]


def render_flags(name: str, ns: str, source: str, flags: str, extra: dict) -> str:
    magic_block = ""
    if "magic" in extra:
        m = extra["magic"]
        magic_block = f"""
[[nodiscard]] inline bool file_magic(const char* path) noexcept {{
    (void)path;
    return true; // stub — magic {m!r}
}}
"""
    return f"""module;
#include <cstddef>

export module pbsd.userland.{name};

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from {source} — flag parse scaffold (logic-only).
export namespace pbsd::userland::{ns}::{name.replace("-", "_")} {{

struct Options {{
    bool force{{false}};
    bool quiet{{false}};
    bool verbose{{false}};
}};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {{
    Options opt{{}};
    if (argv == nullptr) {{
        return result_err<Options>(Status::Invalid);
    }}
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {{
        const char* arg = argv[i];
        if (hosted::cstrcmp(arg, "--") == 0) {{
            ++i;
            break;
        }}
        for (int j = 1; arg[j] != '\\0'; ++j) {{
            switch (arg[j]) {{
            case 'f':
                opt.force = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            default:
                break;
            }}
        }}
    }}
    optind_out = i;
    return result_ok(opt);
}}

[[nodiscard]] inline bool accepts_flag(char c) noexcept {{
    const char* allowed = "{flags}";
    for (const char* p = allowed; *p; ++p) {{
        if (*p == c) {{
            return true;
        }}
    }}
    return false;
}}
{magic_block}}} // namespace pbsd::userland::{ns}::{name.replace("-", "_")}
"""


def render_subcmd(name: str, ns: str, source: str, cmds: list[str]) -> str:
    enum_cases = "\n".join(f"    {c.title().replace('_', '')},\n" for c in cmds)
    enum_names = ",\n        ".join(f"{c.title().replace('_', '')}" for c in cmds)
    table_rows = "\n".join(
        f'        {{"{c}", Subcommand::{c.title().replace("_", "")}}},' for c in cmds
    )
    return f"""module;
#include <cstddef>

export module pbsd.userland.{name};

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from {source} — subcommand registry (logic-only).
export namespace pbsd::userland::{ns}::{name.replace("-", "_")} {{

enum class Subcommand : unsigned char {{
{enum_cases}    Unknown,
}};

struct Options {{
    Subcommand cmd{{Subcommand::Unknown}};
    bool recursive{{false}};
    bool force{{false}};
    bool verbose{{false}};
}};

[[nodiscard]] inline Result<Subcommand> subcommand_from_name(const char* name) noexcept {{
    if (name == nullptr) {{
        return result_err<Subcommand>(Status::Invalid);
    }}
    static const struct {{
        const char* name;
        Subcommand cmd;
    }} kTable[] = {{
{table_rows}
        {{nullptr, Subcommand::Unknown}},
    }};
    for (int i = 0; kTable[i].name != nullptr; ++i) {{
        if (hosted::cstrcmp(name, kTable[i].name) == 0) {{
            return result_ok(kTable[i].cmd);
        }}
    }}
    return result_err<Subcommand>(Status::Invalid);
}}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {{
    Options opt{{}};
    if (argv == nullptr || argc < 2) {{
        return result_err<Options>(Status::Invalid);
    }}
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {{
        const char* arg = argv[i];
        if (hosted::cstrcmp(arg, "-r") == 0) {{
            opt.recursive = true;
            continue;
        }}
        if (hosted::cstrcmp(arg, "-f") == 0) {{
            opt.force = true;
            continue;
        }}
        if (hosted::cstrcmp(arg, "-v") == 0) {{
            opt.verbose = true;
            continue;
        }}
        return result_err<Options>(Status::Invalid);
    }}
    if (i >= argc || argv[i] == nullptr) {{
        return result_err<Options>(Status::Invalid);
    }}
    const auto cmd = subcommand_from_name(argv[i]);
    if (!cmd.has_value()) {{
        return result_err<Options>(Status::Invalid);
    }}
    opt.cmd = cmd.value;
    optind_out = i + 1;
    return result_ok(opt);
}}

}} // namespace pbsd::userland::{ns}::{name.replace("-", "_")}
"""


def render_stub(name: str, ns: str, source: str, extra: dict) -> str:
    scheme = extra.get("scheme", "http")
    return f"""module;
#include <cstddef>

export module pbsd.userland.{name};

export import pbsd.core;
export import pbsd.userland.hosted;

/// Stub helpers from {source} — hosted-network deferred (logic-only).
export namespace pbsd::userland::{ns}::{name.replace("-", "_")} {{

struct Options {{
    const char* url{{nullptr}};
    bool quiet{{false}};
    bool verbose{{false}};
}};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {{
    Options opt{{}};
    if (argv == nullptr) {{
        return result_err<Options>(Status::Invalid);
    }}
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {{
        const char* arg = argv[i];
        for (int j = 1; arg[j] != '\\0'; ++j) {{
            if (arg[j] == 'q') {{
                opt.quiet = true;
            }} else if (arg[j] == 'v') {{
                opt.verbose = true;
            }}
        }}
    }}
    if (i < argc && argv[i] != nullptr) {{
        opt.url = argv[i];
        ++i;
    }}
    optind_out = i;
    return result_ok(opt);
}}

[[nodiscard]] inline bool url_has_scheme(const char* url) noexcept {{
    if (url == nullptr) {{
        return false;
    }}
    const char* p = url;
    while (*p && *p != ':') {{
        ++p;
    }}
    return p > url && p[0] == ':' && p[1] == '/';
}}

[[nodiscard]] inline bool default_scheme_is_{scheme}() noexcept {{
    return true;
}}

}} // namespace pbsd::userland::{ns}::{name.replace("-", "_")}
"""


def render_module(tool: tuple) -> str:
    name, _cat, ns, source, kind, extra = tool
    if kind == "flags":
        return render_flags(name, ns, source, extra.get("flags", ""), extra)
    if kind == "subcmd":
        return render_subcmd(name, ns, source, extra["cmds"])
    return render_stub(name, ns, source, extra)


def main() -> None:
    written: list[str] = []
    for tool in TOOLS:
        name, cat_dir, *_ = tool
        out = USERLAND / cat_dir / f"pbsd.userland.{name}.cppm"
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(render_module(tool), encoding="utf-8")
        written.append(name)
    print(f"Wrote {len(written)} modules: {', '.join(written)}")


if __name__ == "__main__":
    main()
