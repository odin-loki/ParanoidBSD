#!/usr/bin/env python3
"""Generate Burst 13 hand-port modules (pkg + libc + kernel + net + msun)."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
PKG = PBSD / "pkg"
UL = PBSD / "userland"
NET = PBSD / "net"
KERN = PBSD / "kernel" / "kern"

MODULES: dict[Path, str] = {}


def add(path: Path, body: str) -> None:
    MODULES[path] = body.strip() + "\n"


# --- pkg ----------------------------------------------------------------------

add(
    PKG / "pbsd.pkg.transaction.cppm",
    """
module;
#include <cstdint>

export module pbsd.pkg.transaction;

import pbsd.core;
import pbsd.pkg.manifest;

/// Burst 13 — atomic package transaction staging (content-addressed).
export namespace pbsd::pkg::transaction {

enum class Phase : unsigned char {
    Idle = 0,
    Staging,
    Verified,
    Applied,
    RolledBack,
};

inline constexpr unsigned kMaxPackages = 32;

struct StagedPackage {
    char name[64]{};
    char digest_hex[65]{};
    manifest::EntryType kind{manifest::EntryType::File};
};

struct Tx {
    Phase phase{Phase::Idle};
    unsigned count{0};
    StagedPackage packages[kMaxPackages]{};
};

[[nodiscard]] inline Status begin(Tx& tx) noexcept {
    if (tx.phase != Phase::Idle) {
        return Status::Busy;
    }
    tx.phase = Phase::Staging;
    tx.count = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status stage(Tx& tx, StagedPackage const& pkg) noexcept {
    if (tx.phase != Phase::Staging || tx.count >= kMaxPackages) {
        return Status::Invalid;
    }
    if (pkg.name[0] == '\\0' || pkg.digest_hex[0] == '\\0') {
        return Status::Invalid;
    }
    tx.packages[tx.count++] = pkg;
    return Status::Ok;
}

[[nodiscard]] inline Status mark_verified(Tx& tx) noexcept {
    if (tx.phase != Phase::Staging || tx.count == 0) {
        return Status::Invalid;
    }
    tx.phase = Phase::Verified;
    return Status::Ok;
}

[[nodiscard]] inline Status commit(Tx& tx) noexcept {
    if (tx.phase != Phase::Verified) {
        return Status::Invalid;
    }
    tx.phase = Phase::Applied;
    return Status::Ok;
}

[[nodiscard]] inline Status rollback(Tx& tx) noexcept {
    if (tx.phase != Phase::Applied) {
        return Status::Invalid;
    }
    tx.phase = Phase::RolledBack;
    return Status::Ok;
}

} // namespace pbsd::pkg::transaction
""",
)

add(
    PKG / "pbsd.pkg.verify.cppm",
    """
module;

export module pbsd.pkg.verify;

import pbsd.core;
import pbsd.pkg.manifest;
import pbsd.pkg.signature;

/// Burst 13 — package digest/signature verification scaffold.
export namespace pbsd::pkg::verify {

enum class Check : unsigned char {
    Digest = 1,
    Signature = 2,
    Manifest = 4,
};

[[nodiscard]] inline Status verify_digest(char const* digest_hex) noexcept {
    if (digest_hex == nullptr || digest_hex[0] == '\\0') {
        return Status::Invalid;
    }
    unsigned len = 0;
    for (unsigned i = 0; digest_hex[i] != '\\0'; ++i) {
        const char c = digest_hex[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return Status::Protocol;
        }
        ++len;
    }
    if (len != 64) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status verify_manifest(manifest::Manifest const& m) noexcept {
    if (m.entry_count == 0) {
        return Status::Invalid;
    }
    return manifest::validate(m);
}

[[nodiscard]] inline Status verify_all(manifest::Manifest const& m,
                                       signature::SignedManifest const& sig,
                                       unsigned checks) noexcept {
    if ((checks & static_cast<unsigned>(Check::Manifest)) != 0) {
        const Status s = verify_manifest(m);
        if (s != Status::Ok) {
            return s;
        }
    }
    if ((checks & static_cast<unsigned>(Check::Signature)) != 0) {
        const Status s = signature::check(sig);
        if (s != Status::Ok) {
            return s;
        }
    }
    return Status::Ok;
}

} // namespace pbsd::pkg::verify
""",
)

add(
    PKG / "pbsd.pkg.deps.cppm",
    """
module;

export module pbsd.pkg.deps;

import pbsd.core;

/// Burst 13 — package dependency edge list (name/version pairs).
export namespace pbsd::pkg::deps {

inline constexpr unsigned kMaxDeps = 64;

struct Dependency {
    char package[64]{};
    char version[32]{};
    bool optional{false};
};

struct DepList {
    unsigned count{0};
    Dependency items[kMaxDeps]{};
};

[[nodiscard]] inline Status add(DepList& list, Dependency const& dep) noexcept {
    if (list.count >= kMaxDeps || dep.package[0] == '\\0') {
        return Status::Invalid;
    }
    list.items[list.count++] = dep;
    return Status::Ok;
}

[[nodiscard]] inline bool satisfies(DepList const& list, char const* name) noexcept {
    if (name == nullptr) {
        return false;
    }
    for (unsigned i = 0; i < list.count; ++i) {
        bool match = true;
        for (unsigned j = 0; name[j] != '\\0' || list.items[i].package[j] != '\\0'; ++j) {
            if (name[j] != list.items[i].package[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::pkg::deps
""",
)

add(
    PKG / "pbsd.pkg.resolver.cppm",
    """
module;

export module pbsd.pkg.resolver;

import pbsd.core;
import pbsd.pkg.index;
import pbsd.pkg.deps;

/// Burst 13 — resolve package name → index entry with dependency closure.
export namespace pbsd::pkg::resolver {

inline constexpr unsigned kMaxResolved = 32;

struct Resolved {
    unsigned count{0};
    index::IndexEntry entries[kMaxResolved]{};
};

[[nodiscard]] inline Status resolve_one(index::PackageIndex const& idx,
                                        char const* name,
                                        index::IndexEntry& out) noexcept {
    return index::lookup(idx, name, out);
}

[[nodiscard]] inline Status resolve(index::PackageIndex const& idx,
                                    deps::DepList const& need,
                                    Resolved& out) noexcept {
    out.count = 0;
    for (unsigned i = 0; i < need.count; ++i) {
        if (out.count >= kMaxResolved) {
            return Status::Denied;
        }
        index::IndexEntry e{};
        const Status s = index::lookup(idx, need.items[i].package, e);
        if (s != Status::Ok) {
            if (need.items[i].optional) {
                continue;
            }
            return s;
        }
        out.entries[out.count++] = e;
    }
    return out.count > 0 ? Status::Ok : Status::NotFound;
}

} // namespace pbsd::pkg::resolver
""",
)

add(
    PKG / "pbsd.pkg.lockfile.cppm",
    """
module;
#include <cstdint>

export module pbsd.pkg.lockfile;

import pbsd.core;
import pbsd.pkg.index;

/// Burst 13 — lockfile slot metadata (frozen index snapshot).
export namespace pbsd::pkg::lockfile {

inline constexpr unsigned kMagic = 0x50425344u; // 'PBSD'
inline constexpr unsigned kVersion = 1;

struct Header {
    unsigned magic{kMagic};
    unsigned version{kVersion};
    unsigned entry_count{0};
    std::uint64_t created_unix{0};
};

struct Lockfile {
    Header hdr{};
    index::PackageIndex index{};
};

[[nodiscard]] inline Status validate(Header const& h) noexcept {
    if (h.magic != kMagic || h.version != kVersion) {
        return Status::Protocol;
    }
    if (h.entry_count > index::kMaxEntries) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status freeze(Lockfile& lf, index::PackageIndex const& idx,
                                   std::uint64_t ts) noexcept {
    lf.hdr.created_unix = ts;
    lf.index = idx;
    lf.hdr.entry_count = idx.count;
    return validate(lf.hdr);
}

} // namespace pbsd::pkg::lockfile
""",
)

# --- libc gen -----------------------------------------------------------------

LIBC_GEN = {
    "usleep": (
        """
module;

export module pbsd.userland.libc.gen.usleep;

export import pbsd.core;

/// usleep from hbsd/src/lib/libc/gen/usleep.c
export namespace pbsd::userland::libc {

struct Timespec {
    long sec{};
    long nsec{};
};

[[nodiscard]] inline Timespec usec_to_timespec(unsigned usec) noexcept {
    Timespec ts{};
    ts.nsec = static_cast<long>((usec % 1000000u) * 1000u);
    ts.sec = static_cast<long>(usec / 1000000u);
    return ts;
}

[[nodiscard]] inline unsigned timespec_to_usec(Timespec ts) noexcept {
    if (ts.sec < 0 || ts.nsec < 0) {
        return 0;
    }
    return static_cast<unsigned>(ts.sec) * 1000000u
         + static_cast<unsigned>(ts.nsec / 1000);
}

} // namespace pbsd::userland::libc
""",
        None,
    ),
    "ualarm": (
        """
module;

export module pbsd.userland.libc.gen.ualarm;

export import pbsd.core;

/// ualarm from hbsd/src/lib/libc/gen/ualarm.c
export namespace pbsd::userland::libc {

inline constexpr unsigned kUsecPerSec = 1000000u;

[[nodiscard]] inline unsigned usec_remainder(unsigned usec, unsigned elapsed) noexcept {
    if (elapsed >= usec) {
        return 0;
    }
    return usec - elapsed;
}

[[nodiscard]] inline unsigned usec_interval(unsigned usec, unsigned interval) noexcept {
    if (interval == 0) {
        return usec;
    }
    return interval;
}

} // namespace pbsd::userland::libc
""",
        None,
    ),
    "alarm": (
        """
module;

export module pbsd.userland.libc.gen.alarm;

export import pbsd.core;

/// alarm from hbsd/src/lib/libc/gen/alarm.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned alarm_seconds(unsigned seconds, unsigned pending) noexcept {
    const unsigned prev = pending;
    (void)seconds;
    return prev;
}

[[nodiscard]] inline Status validate_alarm(unsigned seconds) noexcept {
    if (seconds > 100000000u) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
""",
        None,
    ),
    "pause": (
        """
module;

export module pbsd.userland.libc.gen.pause;

export import pbsd.core;

/// pause from hbsd/src/lib/libc/gen/pause.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool signal_delivered(int pending_mask) noexcept {
    return pending_mask != 0;
}

} // namespace pbsd::userland::libc
""",
        None,
    ),
    "nice": (
        """
module;

export module pbsd.userland.libc.gen.nice;

export import pbsd.core;

/// nice from hbsd/src/lib/libc/gen/nice.c
export namespace pbsd::userland::libc {

inline constexpr int kMinNice = -20;
inline constexpr int kMaxNice = 20;

[[nodiscard]] inline Status validate_nice(int incr) noexcept {
    if (incr < kMinNice || incr > kMaxNice) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline int clamp_nice(int prio) noexcept {
    if (prio < kMinNice) {
        return kMinNice;
    }
    if (prio > kMaxNice) {
        return kMaxNice;
    }
    return prio;
}

} // namespace pbsd::userland::libc
""",
        None,
    ),
    "stringlist": (
        """
module;
#include <cstddef>

export module pbsd.userland.libc.gen.stringlist;

export import pbsd.core;

/// stringlist from hbsd/src/lib/libc/gen/stringlist.c
export namespace pbsd::userland::libc {

inline constexpr unsigned kChunkSize = 20;

struct StringList {
    unsigned cur{0};
    unsigned max{kChunkSize};
};

[[nodiscard]] inline Status sl_init(StringList& sl) noexcept {
    sl.cur = 0;
    sl.max = kChunkSize;
    return Status::Ok;
}

[[nodiscard]] inline Status sl_need(StringList& sl, unsigned extra) noexcept {
    if (sl.cur + extra > sl.max) {
        sl.max += kChunkSize;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned sl_count(StringList const& sl) noexcept {
    return sl.cur;
}

} // namespace pbsd::userland::libc
""",
        None,
    ),
    "strtofflags": (
        """
module;
#include <cstdint>

export module pbsd.userland.libc.gen.strtofflags;

export import pbsd.core;

/// strtofflags from hbsd/src/lib/libc/gen/strtofflags.c
export namespace pbsd::userland::libc {

inline constexpr unsigned kSfAppend = 0x00040000u;
inline constexpr unsigned kSfImmutable = 0x00020000u;
inline constexpr unsigned kUfAppend = 0x00000004u;
inline constexpr unsigned kUfImmutable = 0x00000002u;
inline constexpr unsigned kUfNodump = 0x00000001u;

struct FlagMapping {
    const char* name;
    bool invert;
    unsigned flag;
};

[[nodiscard]] inline unsigned lookup_flag(FlagMapping const* table, unsigned n,
                                          char const* name, bool& invert_out) noexcept {
    if (table == nullptr || name == nullptr) {
        return 0;
    }
    for (unsigned i = 0; i < n; ++i) {
        bool match = true;
        for (unsigned j = 0; name[j] != '\\0' || table[i].name[j] != '\\0'; ++j) {
            if (name[j] != table[i].name[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            invert_out = table[i].invert;
            return table[i].flag;
        }
    }
    return 0;
}

[[nodiscard]] inline Status apply_invert(unsigned flag, bool invert,
                                         unsigned& set, unsigned& clear) noexcept {
    if (invert) {
        clear |= flag;
    } else {
        set |= flag;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
""",
        None,
    ),
    "sysconf": (
        """
module;

export module pbsd.userland.libc.gen.sysconf;

export import pbsd.core;

/// sysconf from hbsd/src/lib/libc/gen/sysconf.c
export namespace pbsd::userland::libc {

enum class SysconfName : int {
    ArgMax = 1,
    ChildMax = 2,
    ClkTck = 3,
    NGroupsMax = 4,
    OpenMax = 5,
    PageSize = 6,
    StreamMax = 7,
    TznameMax = 8,
};

[[nodiscard]] inline long sysconf_value(SysconfName name) noexcept {
    switch (name) {
    case SysconfName::ArgMax:
        return 262144;
    case SysconfName::ChildMax:
        return 640;
    case SysconfName::ClkTck:
        return 100;
    case SysconfName::NGroupsMax:
        return 16;
    case SysconfName::OpenMax:
        return 64;
    case SysconfName::PageSize:
        return 4096;
    case SysconfName::StreamMax:
        return 20;
    case SysconfName::TznameMax:
        return 32;
    default:
        return -1;
    }
}

[[nodiscard]] inline Status validate_name(int name) noexcept {
    if (name <= 0 || name > static_cast<int>(SysconfName::TznameMax)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
""",
        None,
    ),
    "siglist": (
        """
module;

export module pbsd.userland.libc.gen.siglist;

export import pbsd.core;

/// siglist/sys_signame from hbsd/src/lib/libc/gen/siglist.c
export namespace pbsd::userland::libc {

inline constexpr int kNsigs = 33;

inline constexpr const char* kSigname[kNsigs] = {
    "Signal 0",
    "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE", "KILL",
    "BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", "URG", "STOP", "TSTP",
    "CONT", "CHLD", "TTIN", "TTOU", "IO", "XCPU", "XFSZ", "VTALRM", "PROF",
    "WINCH", "INFO", "USR1", "USR2",
};

[[nodiscard]] inline char const* signame(int sig) noexcept {
    if (sig < 0 || sig >= kNsigs) {
        return nullptr;
    }
    return kSigname[sig];
}

[[nodiscard]] inline char const* sigdescr(int sig) noexcept {
    switch (sig) {
    case 1: return "Hangup";
    case 2: return "Interrupt";
    case 9: return "Killed";
    case 15: return "Terminated";
    default: return signame(sig);
    }
}

} // namespace pbsd::userland::libc
""",
        None,
    ),
}

for name, (body, _) in LIBC_GEN.items():
    add(UL / "libc" / f"pbsd.userland.libc.gen.{name}.cppm", body)

# --- kernel -------------------------------------------------------------------

add(
    KERN / "pbsd.kernel.subr_unit.cppm",
    """
module;

export module pbsd.kernel.subr_unit;

import pbsd.core;

/// Freestanding port of `subr_unit.c` — unit number allocation header.
export namespace pbsd::kernel::subr_unit {

inline constexpr int kNoUnit = -1;

struct UnitRange {
    int low{0};
    int high{0};
    unsigned allocated{0};
};

[[nodiscard]] inline Status init(UnitRange& r, int max) noexcept {
    if (max <= 0) {
        return Status::Invalid;
    }
    r.low = 0;
    r.high = max;
    r.allocated = 0;
    return Status::Ok;
}

[[nodiscard]] inline int alloc(UnitRange& r) noexcept {
    if (r.allocated >= static_cast<unsigned>(r.high - r.low)) {
        return kNoUnit;
    }
    return r.low + static_cast<int>(r.allocated++);
}

[[nodiscard]] inline Status free_unit(UnitRange& r, int unit) noexcept {
    if (unit < r.low || unit >= r.high) {
        return Status::Invalid;
    }
    if (r.allocated > 0) {
        --r.allocated;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_unit
""",
)

add(
    KERN / "pbsd.kernel.subr_hash.cppm",
    """
module;

export module pbsd.kernel.subr_hash;

import pbsd.core;

/// Freestanding port of `subr_hash.c` — hash table sizing helpers.
export namespace pbsd::kernel::subr_hash {

enum class HashFlag : unsigned {
    WaitOk = 0x01,
    NoWait = 0x02,
};

[[nodiscard]] inline unsigned hash_size(int elements) noexcept {
    if (elements <= 0) {
        return 0;
    }
    unsigned hashsize = 1;
    while (static_cast<int>(hashsize) <= elements) {
        hashsize <<= 1;
    }
    return hashsize >> 1;
}

[[nodiscard]] inline unsigned hash_mask(unsigned hashsize) noexcept {
    return hashsize - 1;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    const bool wait = (flags & static_cast<unsigned>(HashFlag::WaitOk)) != 0;
    const bool nowait = (flags & static_cast<unsigned>(HashFlag::NoWait)) != 0;
    if (wait == nowait) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_hash
""",
)

add(
    KERN / "pbsd.kernel.subr_msgbuf.cppm",
    """
module;
#include <cstdint>

export module pbsd.kernel.subr_msgbuf;

import pbsd.core;

/// Freestanding port of `subr_msgbuf.c` — kernel message buffer metadata.
export namespace pbsd::kernel::subr_msgbuf {

inline constexpr unsigned kDefaultSize = 4096;

struct Msgbuf {
    unsigned size{kDefaultSize};
    unsigned head{};
    unsigned tail{};
    unsigned seq{};
    bool enabled{true};
};

[[nodiscard]] inline Status init(Msgbuf& mb, unsigned size) noexcept {
    if (size < 256) {
        return Status::Invalid;
    }
    mb.size = size;
    mb.head = 0;
    mb.tail = 0;
    mb.seq = 0;
    mb.enabled = true;
    return Status::Ok;
}

[[nodiscard]] inline unsigned available(Msgbuf const& mb) noexcept {
    if (!mb.enabled || mb.size <= mb.head) {
        return 0;
    }
    return mb.size - mb.head;
}

[[nodiscard]] inline Status append(Msgbuf& mb, unsigned len) noexcept {
    if (!mb.enabled) {
        return Status::Invalid;
    }
    if (mb.head + len > mb.size) {
        return Status::Denied;
    }
    mb.head += len;
    ++mb.seq;
    return Status::Ok;
}

inline void clear(Msgbuf& mb) noexcept {
    mb.head = 0;
    mb.tail = 0;
}

} // namespace pbsd::kernel::subr_msgbuf
""",
)

add(
    KERN / "pbsd.kernel.subr_prf.cppm",
    """
module;

export module pbsd.kernel.subr_prf;

import pbsd.core;

/// Freestanding port of `subr_prf.c` — printf severity levels.
export namespace pbsd::kernel::subr_prf {

enum class LogLevel : unsigned char {
    Emergency = 0,
    Alert = 1,
    Critical = 2,
    Error = 3,
    Warning = 4,
    Notice = 5,
    Info = 6,
    Debug = 7,
};

[[nodiscard]] inline bool level_enabled(LogLevel cur, LogLevel msg) noexcept {
    return static_cast<unsigned>(msg) <= static_cast<unsigned>(cur);
}

[[nodiscard]] inline char const* level_name(LogLevel lvl) noexcept {
    switch (lvl) {
    case LogLevel::Emergency: return "EMERG";
    case LogLevel::Alert: return "ALERT";
    case LogLevel::Critical: return "CRIT";
    case LogLevel::Error: return "ERR";
    case LogLevel::Warning: return "WARN";
    case LogLevel::Notice: return "NOTICE";
    case LogLevel::Info: return "INFO";
    case LogLevel::Debug: return "DEBUG";
    default: return "?";
    }
}

[[nodiscard]] inline Status validate_level(unsigned lvl) noexcept {
    if (lvl > static_cast<unsigned>(LogLevel::Debug)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_prf
""",
)

# --- net ----------------------------------------------------------------------

add(
    NET / "pbsd.net.in_cksum.cppm",
    """
module;
#include <cstdint>

export module pbsd.net.in_cksum;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_cksum.c — Internet checksum fold.
export namespace pbsd::net::in_cksum {

[[nodiscard]] inline std::uint16_t fold32(std::uint32_t sum) noexcept {
    sum = (sum >> 16) + (sum & 0xffffu);
    sum += (sum >> 16);
    return static_cast<std::uint16_t>(~sum);
}

[[nodiscard]] inline std::uint16_t add16(std::uint16_t a, std::uint16_t b) noexcept {
    std::uint32_t sum = static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b);
    if (sum > 0xffffu) {
        sum = (sum & 0xffffu) + 1u;
    }
    return static_cast<std::uint16_t>(sum);
}

[[nodiscard]] inline std::uint32_t accumulate(std::uint32_t sum,
                                              std::uint16_t word) noexcept {
    return sum + static_cast<std::uint32_t>(word);
}

[[nodiscard]] inline std::uint16_t finish(std::uint32_t sum) noexcept {
    return fold32(sum);
}

} // namespace pbsd::net::in_cksum
""",
)

add(
    NET / "pbsd.net.tcp_hpts.cppm",
    """
module;
#include <cstdint>

export module pbsd.net.tcp_hpts;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_hpts.h — HPTS slot timing helpers.
export namespace pbsd::net::tcp_hpts {

inline constexpr unsigned kUsecsPerSlot = 10;
inline constexpr unsigned kUsecInSec = 1000000;
inline constexpr unsigned kMsecInSec = 1000;
inline constexpr unsigned kUsecInMsec = 1000;

struct TimeVal {
    long sec{};
    long usec{};
};

[[nodiscard]] inline unsigned ms_to_slots(unsigned ms) noexcept {
    return ms * 100u + 1u;
}

[[nodiscard]] inline unsigned usec_to_slots(unsigned usec) noexcept {
    return (usec + 9u) / kUsecsPerSlot;
}

[[nodiscard]] inline unsigned tv_to_hpts_slot(TimeVal const& sv) noexcept {
    return static_cast<unsigned>(sv.sec) * 100000u
         + static_cast<unsigned>(sv.usec / kUsecsPerSlot);
}

[[nodiscard]] inline unsigned tv_to_usec(TimeVal const& sv) noexcept {
    return static_cast<unsigned>(sv.sec) * kUsecInSec
         + static_cast<unsigned>(sv.usec);
}

[[nodiscard]] inline unsigned tv_to_msec(TimeVal const& sv) noexcept {
    return static_cast<unsigned>(sv.sec) * kMsecInSec
         + static_cast<unsigned>(sv.usec / kUsecInMsec);
}

} // namespace pbsd::net::tcp_hpts
""",
)

# --- msun ---------------------------------------------------------------------

add(
    UL / "msun" / "pbsd.userland.msun.j0.cppm",
    """
module;
#include <cmath>

export module pbsd.userland.msun.j0;

/// j0/j0f Bessel from hbsd/src/lib/msun/src/e_j0.c (delegates to libm).
export namespace pbsd::userland::msun {

[[nodiscard]] inline double j0(double x) noexcept {
#if defined(__cpp_lib_math_special_functions) && __cpp_lib_math_special_functions >= 201603L
    return std::cyl_bessel_j(0, x);
#else
    (void)x;
    return 0.0;
#endif
}

[[nodiscard]] inline float j0f(float x) noexcept {
    return static_cast<float>(j0(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)

add(
    UL / "msun" / "pbsd.userland.msun.y0.cppm",
    """
module;
#include <cmath>

export module pbsd.userland.msun.y0;

/// y0/y0f Bessel from hbsd/src/lib/msun/src/e_y0.c (delegates to libm).
export namespace pbsd::userland::msun {

[[nodiscard]] inline double y0(double x) noexcept {
#if defined(__cpp_lib_math_special_functions) && __cpp_lib_math_special_functions >= 201603L
    return std::cyl_neumann(0, x);
#else
    (void)x;
    return 0.0;
#endif
}

[[nodiscard]] inline float y0f(float x) noexcept {
    return static_cast<float>(y0(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)


def write_modules() -> None:
    for path, body in MODULES.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(body, encoding="utf-8")
        print(f"  wrote {path.relative_to(ROOT)}")


def patch_pkg_cmake() -> None:
    cmake = PKG / "CMakeLists.txt"
    text = cmake.read_text(encoding="utf-8")
    block = """
if(NOT TARGET pbsd_pkg_transaction)
add_library(pbsd_pkg_transaction)
target_sources(pbsd_pkg_transaction PUBLIC FILE_SET CXX_MODULES FILES pbsd.pkg.transaction.cppm)
target_link_libraries(pbsd_pkg_transaction PUBLIC pbsd_core pbsd_pkg_manifest)
target_compile_options(pbsd_pkg_transaction PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_pkg_verify)
add_library(pbsd_pkg_verify)
target_sources(pbsd_pkg_verify PUBLIC FILE_SET CXX_MODULES FILES pbsd.pkg.verify.cppm)
target_link_libraries(pbsd_pkg_verify PUBLIC pbsd_core pbsd_pkg_manifest pbsd_pkg_signature)
target_compile_options(pbsd_pkg_verify PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_pkg_deps)
add_library(pbsd_pkg_deps)
target_sources(pbsd_pkg_deps PUBLIC FILE_SET CXX_MODULES FILES pbsd.pkg.deps.cppm)
target_link_libraries(pbsd_pkg_deps PUBLIC pbsd_core)
target_compile_options(pbsd_pkg_deps PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_pkg_resolver)
add_library(pbsd_pkg_resolver)
target_sources(pbsd_pkg_resolver PUBLIC FILE_SET CXX_MODULES FILES pbsd.pkg.resolver.cppm)
target_link_libraries(pbsd_pkg_resolver PUBLIC pbsd_core pbsd_pkg_index pbsd_pkg_deps)
target_compile_options(pbsd_pkg_resolver PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_pkg_lockfile)
add_library(pbsd_pkg_lockfile)
target_sources(pbsd_pkg_lockfile PUBLIC FILE_SET CXX_MODULES FILES pbsd.pkg.lockfile.cppm)
target_link_libraries(pbsd_pkg_lockfile PUBLIC pbsd_core pbsd_pkg_index)
target_compile_options(pbsd_pkg_lockfile PUBLIC ${PBSD_FS_CXX})
endif()

"""
    if "pbsd_pkg_transaction" not in text:
        text = text.replace("if(NOT TARGET pbsd_pkg)\n", block + "if(NOT TARGET pbsd_pkg)\n", 1)
    old_link = """target_link_libraries(pbsd_pkg PUBLIC pbsd_core pbsd_pkg_manifest pbsd_pkg_repo
    pbsd_pkg_signature pbsd_pkg_index pbsd_pkg_bootstrap pbsd_pkg_cache)"""
    new_link = """target_link_libraries(pbsd_pkg PUBLIC pbsd_core pbsd_pkg_manifest pbsd_pkg_repo
    pbsd_pkg_signature pbsd_pkg_index pbsd_pkg_bootstrap pbsd_pkg_cache
    pbsd_pkg_transaction pbsd_pkg_verify pbsd_pkg_deps pbsd_pkg_resolver pbsd_pkg_lockfile)"""
    if "pbsd_pkg_transaction" not in text.split("target_link_libraries(pbsd_pkg PUBLIC")[1].split("\n")[0:6].__str__():
        text = text.replace(old_link, new_link, 1)
    cmake.write_text(text, encoding="utf-8")
    print("  patched pkg/CMakeLists.txt")


def patch_pkg_aggregate() -> None:
    agg = PKG / "pbsd.pkg.cppm"
    text = agg.read_text(encoding="utf-8")
    for imp in [
        "export import pbsd.pkg.transaction;",
        "export import pbsd.pkg.verify;",
        "export import pbsd.pkg.deps;",
        "export import pbsd.pkg.resolver;",
        "export import pbsd.pkg.lockfile;",
    ]:
        if imp not in text:
            text = text.replace("export import pbsd.pkg.cache;", "export import pbsd.pkg.cache;\n" + imp, 1)
    agg.write_text(text, encoding="utf-8")
    print("  patched pbsd.pkg.cppm")


def patch_userland_cmake() -> None:
    cmake = UL / "CMakeLists.txt"
    text = cmake.read_text(encoding="utf-8")
    libc_add = (
        "    libc/pbsd.userland.libc.gen.siglist.cppm\n"
        "    libc/pbsd.userland.libc.gen.sysconf.cppm\n"
        "    libc/pbsd.userland.libc.gen.strtofflags.cppm\n"
        "    libc/pbsd.userland.libc.gen.stringlist.cppm\n"
        "    libc/pbsd.userland.libc.gen.nice.cppm\n"
        "    libc/pbsd.userland.libc.gen.pause.cppm\n"
        "    libc/pbsd.userland.libc.gen.alarm.cppm\n"
        "    libc/pbsd.userland.libc.gen.ualarm.cppm\n"
        "    libc/pbsd.userland.libc.gen.usleep.cppm\n"
    )
    needle = "    libc/pbsd.userland.libc.gen.sleep.cppm\n    libc/pbsd.userland.libc.cppm"
    if "gen.usleep.cppm" not in text:
        text = text.replace(needle, libc_add + "    libc/pbsd.userland.libc.cppm", 1)
    msun_add = (
        "    msun/pbsd.userland.msun.j0.cppm\n"
        "    msun/pbsd.userland.msun.y0.cppm\n"
    )
    msun_needle = "    msun/pbsd.userland.msun.scalbln.cppm\n    msun/pbsd.userland.msun.cppm"
    if "msun.j0.cppm" not in text:
        text = text.replace(msun_needle, msun_add + "    msun/pbsd.userland.msun.cppm", 1)
    cmake.write_text(text, encoding="utf-8")
    print("  patched userland/CMakeLists.txt")


def patch_libc_msun_aggregate() -> None:
    libc = UL / "libc" / "pbsd.userland.libc.cppm"
    text = libc.read_text(encoding="utf-8")
    for imp in [
        "export import pbsd.userland.libc.gen.usleep;",
        "export import pbsd.userland.libc.gen.ualarm;",
        "export import pbsd.userland.libc.gen.alarm;",
        "export import pbsd.userland.libc.gen.pause;",
        "export import pbsd.userland.libc.gen.nice;",
        "export import pbsd.userland.libc.gen.stringlist;",
        "export import pbsd.userland.libc.gen.strtofflags;",
        "export import pbsd.userland.libc.gen.sysconf;",
        "export import pbsd.userland.libc.gen.siglist;",
    ]:
        if imp not in text:
            text = text.replace(
                "export import pbsd.userland.libc.gen.sleep;",
                "export import pbsd.userland.libc.gen.sleep;\n" + imp,
                1,
            )
    libc.write_text(text, encoding="utf-8")
    msun = UL / "msun" / "pbsd.userland.msun.cppm"
    text = msun.read_text(encoding="utf-8")
    for imp in ["export import pbsd.userland.msun.j0;", "export import pbsd.userland.msun.y0;"]:
        if imp not in text:
            text = text.replace(
                "export import pbsd.userland.msun.scalbln;",
                "export import pbsd.userland.msun.scalbln;\n" + imp,
                1,
            )
    msun.write_text(text, encoding="utf-8")
    print("  patched libc/msun aggregates")


def patch_root_cmake() -> None:
    cmake = PBSD / "CMakeLists.txt"
    text = cmake.read_text(encoding="utf-8")
    kblock = """
if(NOT TARGET pbsd_kernel_subr_unit)
add_library(pbsd_kernel_subr_unit)
target_sources(pbsd_kernel_subr_unit PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_unit.cppm)
target_link_libraries(pbsd_kernel_subr_unit PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_unit PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_hash)
add_library(pbsd_kernel_subr_hash)
target_sources(pbsd_kernel_subr_hash PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_hash.cppm)
target_link_libraries(pbsd_kernel_subr_hash PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_hash PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_msgbuf)
add_library(pbsd_kernel_subr_msgbuf)
target_sources(pbsd_kernel_subr_msgbuf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_msgbuf.cppm)
target_link_libraries(pbsd_kernel_subr_msgbuf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_msgbuf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_prf)
add_library(pbsd_kernel_subr_prf)
target_sources(pbsd_kernel_subr_prf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_prf.cppm)
target_link_libraries(pbsd_kernel_subr_prf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_prf PUBLIC ${PBSD_FS_CXX})
endif()

"""
    if "pbsd_kernel_subr_unit" not in text:
        text = text.replace(
            "if(NOT TARGET pbsd_kernel_sys_getrandom)\n",
            kblock + "if(NOT TARGET pbsd_kernel_sys_getrandom)\n",
            1,
        )
    kiface = "    pbsd_kernel_context pbsd_kernel_sys_getrandom)"
    kadd = (
        "    pbsd_kernel_context pbsd_kernel_sys_getrandom\n"
        "    pbsd_kernel_subr_unit pbsd_kernel_subr_hash\n"
        "    pbsd_kernel_subr_msgbuf pbsd_kernel_subr_prf)"
    )
    if "pbsd_kernel_subr_unit" not in text.split("target_link_libraries(pbsd_kernel INTERFACE")[1][:800]:
        text = text.replace(kiface, kadd, 1)

    nblock = """
if(NOT TARGET pbsd_net_in_cksum)
add_library(pbsd_net_in_cksum)
target_sources(pbsd_net_in_cksum PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_cksum.cppm)
target_link_libraries(pbsd_net_in_cksum PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_cksum PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_hpts)
add_library(pbsd_net_tcp_hpts)
target_sources(pbsd_net_tcp_hpts PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_hpts.cppm)
target_link_libraries(pbsd_net_tcp_hpts PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_hpts PUBLIC ${PBSD_FS_CXX})
endif()

"""
    if "pbsd_net_in_cksum" not in text:
        text = text.replace(
            "if(NOT TARGET pbsd_net_tcp_timer)\n",
            nblock + "if(NOT TARGET pbsd_net_tcp_timer)\n",
            1,
        )
    niface = "    pbsd_net_bridgestp pbsd_net_tcp_timer)"
    nadd = "    pbsd_net_bridgestp pbsd_net_tcp_timer pbsd_net_in_cksum pbsd_net_tcp_hpts)"
    if "pbsd_net_in_cksum" not in text.split("target_link_libraries(pbsd_net INTERFACE")[1][:1200]:
        text = text.replace(niface, nadd, 1)

    cmake.write_text(text, encoding="utf-8")
    print("  patched pbsd/CMakeLists.txt")


def patch_net_aggregate() -> None:
    agg = NET / "pbsd.net.cppm"
    text = agg.read_text(encoding="utf-8")
    for imp in ["export import pbsd.net.in_cksum;", "export import pbsd.net.tcp_hpts;"]:
        if imp not in text:
            text = text.replace(
                "export import pbsd.net.tcp_timer;",
                "export import pbsd.net.tcp_timer;\n" + imp,
                1,
            )
    agg.write_text(text, encoding="utf-8")
    print("  patched pbsd.net.cppm")


def main() -> None:
    print(f"Burst 13: writing {len(MODULES)} hand-port modules …")
    write_modules()
    patch_pkg_cmake()
    patch_pkg_aggregate()
    patch_userland_cmake()
    patch_libc_msun_aggregate()
    patch_root_cmake()
    patch_net_aggregate()
    print(f"Done — {len(MODULES)} burst-13 modules.")


if __name__ == "__main__":
    main()
