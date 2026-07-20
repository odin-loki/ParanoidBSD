#!/usr/bin/env python3
"""Patch pbsd/userland/CMakeLists.txt for burst 13 modules."""
from pathlib import Path

path = Path(__file__).resolve().parent.parent / "pbsd" / "userland" / "CMakeLists.txt"
cmake = path.read_text(encoding="utf-8")

b13_libc = [
    "gen.times", "gen.closedir", "gen.readdir", "gen.rewinddir", "gen.telldir", "gen.seekdir",
    "gen.getentropy", "gen.arc4random_uniform", "gen.setmode", "gen.syslog",
    "stdlib.atoi", "stdlib.atol", "stdlib.atoll", "stdlib.qsort", "stdlib.realpath",
    "stdlib.reallocarray", "stdlib.labs", "stdlib.ldiv", "stdlib.abort",
    "stdio.feof", "stdio.ferror", "stdio.ftell", "stdio.rewind", "stdio.getline", "stdio.clrerr",
    "net.base64", "net.gai_strerror", "net.if_nametoindex",
    "sys.open", "sys.read", "sys.write", "sys.close", "sys.fork",
    "locale.rpmatch", "locale.tolower", "locale.toupper",
]
b13_libthr = [
    "self", "create", "fork", "getthreadid_np", "getschedparam", "setschedparam",
    "getprio", "setprio", "getcpuclockid", "info", "barrierattr", "rwlockattr", "pshared",
    "multi_np", "single_np", "switch_np", "suspend_np", "resume_np", "main_np",
    "sig", "sigqueue", "autoinit", "clean", "event", "list", "kern", "malloc", "printf",
    "rtld", "sleepq", "spec", "syscalls", "umtx", "ctrdtr", "pspinlock",
]
b13_usr = [
    "host", "last", "locale", "revoke", "limits", "logins", "fstat", "ident", "mkdep",
    "rpcinfo", "rwall", "rwho", "rup", "ruptime", "rusers", "lastcomm", "runat", "quota",
    "rctl", "newgrp", "readelf", "gencat", "mkstr",
]
b13_bin = ["rmail"]

libc_insert = "\n".join(
    f"    libc/pbsd.userland.libc.{s}.cppm"
    for s in b13_libc
    if f"libc/pbsd.userland.libc.{s}.cppm" not in cmake
)
if libc_insert:
    cmake = cmake.replace(
        "    libc/pbsd.userland.libc.cppm)",
        libc_insert + "\n    libc/pbsd.userland.libc.cppm)",
        1,
    )

libthr_insert = "\n".join(
    f"    libthr/pbsd.userland.libthr.{s}.cppm"
    for s in b13_libthr
    if f"libthr/pbsd.userland.libthr.{s}.cppm" not in cmake
)
if libthr_insert:
    cmake = cmake.replace(
        "    libthr/pbsd.userland.libthr.cppm)",
        libthr_insert + "\n    libthr/pbsd.userland.libthr.cppm)",
        1,
    )

if "burst 13" not in cmake:
    usr_items = " ".join(b13_usr)
    bin_items = " ".join(b13_bin)
    burst13 = f"""
# --- burst 13: libc/libthr gaps + usr.bin/bin utilities ----------------------
foreach(_ul_b13_usr IN ITEMS {usr_items})
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b13_usr}} usr.bin/pbsd.userland.${{_ul_b13_usr}}.cppm)
endforeach()

foreach(_ul_b13_bin IN ITEMS {bin_items})
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b13_bin}} bin/pbsd.userland.${{_ul_b13_bin}}.cppm)
endforeach()
"""
    cmake = cmake.replace("\nif(NOT TARGET pbsd_userland_bin)", burst13 + "\nif(NOT TARGET pbsd_userland_bin)", 1)

usr_block = cmake.split("pbsd_userland_usr_bin INTERFACE")[1].split("endif()")[0]
missing_usr = [t for t in b13_usr if f"pbsd_userland_{t}" not in usr_block]
if missing_usr:
    cmake = cmake.replace(
        "    pbsd_userland_tset)\nendif()",
        "    pbsd_userland_tset\n    "
        + "\n    ".join(f"pbsd_userland_{t}" for t in missing_usr)
        + ")\nendif()",
        1,
    )

bin_block = cmake.split("pbsd_userland_bin INTERFACE")[1].split("endif()")[0]
if "pbsd_userland_rmail" not in bin_block:
    cmake = cmake.replace(
        "    pbsd_userland_umask)\nendif()",
        "    pbsd_userland_umask\n    pbsd_userland_rmail)\nendif()",
        1,
    )

path.write_text(cmake, encoding="utf-8", newline="\n")
print("CMakeLists.txt updated")
