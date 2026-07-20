#!/usr/bin/env python3
"""Burst 13 — net/fs/geom/zfs hand port modules."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd"

MODULES = {
"net/pbsd.net.tcp_ecn.cppm": """module;
#include <cstdint>

export module pbsd.net.tcp_ecn;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_ecn.h — TCP ECN flag helpers.
export namespace pbsd::net::tcp_ecn {

inline constexpr std::uint8_t kThEce = 0x40;
inline constexpr std::uint8_t kThCwr = 0x80;
inline constexpr unsigned kAceShift = 6;

enum class Codepoint : std::uint8_t {
    NotEct = 0,
    Ect1   = 1,
    Ect0   = 2,
    Ce     = 3,
};

[[nodiscard]] inline Status validate_codepoint(Codepoint cp) noexcept {
    return static_cast<unsigned>(cp) <= static_cast<unsigned>(Codepoint::Ce)
        ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline std::uint8_t tos_codepoint(std::uint8_t tos) noexcept {
    return static_cast<std::uint8_t>(tos & 0x03);
}

[[nodiscard]] inline bool is_ce(std::uint8_t tos) noexcept {
    return tos_codepoint(tos) == static_cast<std::uint8_t>(Codepoint::Ce);
}

[[nodiscard]] inline std::uint8_t merge_flags(std::uint8_t flags, bool ece, bool cwr) noexcept {
    auto out = static_cast<std::uint8_t>(flags & ~(kThEce | kThCwr));
    if (ece) {
        out = static_cast<std::uint8_t>(out | kThEce);
    }
    if (cwr) {
        out = static_cast<std::uint8_t>(out | kThCwr);
    }
    return out;
}

} // namespace pbsd::net::tcp_ecn
""",
"net/pbsd.net.tcp_reass.cppm": """module;
#include <cstdint>

export module pbsd.net.tcp_reass;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_var.h — TCP reassembly queue limits.
export namespace pbsd::net::tcp_reass {

inline constexpr unsigned kMaxQueueLen = 100;
inline constexpr unsigned kMaxSegLen = 65535;

struct QueueStats {
    unsigned queued{};
    unsigned dropped{};
    unsigned merged{};
};

[[nodiscard]] inline Status validate_seg_len(unsigned len) noexcept {
    if (len == 0 || len > kMaxSegLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status admit(QueueStats& qs, unsigned len) noexcept {
    if (validate_seg_len(len) != Status::Ok) {
        return Status::Invalid;
    }
    if (qs.queued >= kMaxQueueLen) {
        ++qs.dropped;
        return Status::Busy;
    }
    ++qs.queued;
    return Status::Ok;
}

[[nodiscard]] inline Status release(QueueStats& qs) noexcept {
    if (qs.queued == 0) {
        return Status::Invalid;
    }
    --qs.queued;
    return Status::Ok;
}

} // namespace pbsd::net::tcp_reass
""",
"net/pbsd.net.tcp_sack.cppm": """module;
#include <cstdint>

export module pbsd.net.tcp_sack;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp.h — TCP SACK option layout.
export namespace pbsd::net::tcp_sack {

inline constexpr unsigned kOptKind = 5;
inline constexpr unsigned kOptLen = 10;
inline constexpr unsigned kMaxBlocks = 4;

struct Block {
    std::uint32_t left{};
    std::uint32_t right{};
};

[[nodiscard]] inline Status validate_block(const Block& b) noexcept {
    if (b.right <= b.left) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_count(unsigned count) noexcept {
    if (count == 0 || count > kMaxBlocks) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned option_len(unsigned block_count) noexcept {
    if (validate_count(block_count) != Status::Ok) {
        return 0;
    }
    return 2 + block_count * kOptLen;
}

} // namespace pbsd::net::tcp_sack
""",
"net/pbsd.net.tcp_syncache.cppm": """module;
#include <cstdint>

export module pbsd.net.tcp_syncache;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_syncache.h — SYN cache bucket limits.
export namespace pbsd::net::tcp_syncache {

inline constexpr unsigned kHashSize = 512;
inline constexpr unsigned kBucketLimit = 30;
inline constexpr unsigned kRexmtLimit = 3;

enum class Flag : unsigned short {
    Offload = 0x0001,
    Tfo     = 0x0002,
    Signed  = 0x0004,
};

struct Bucket {
    unsigned count{};
    unsigned rexmt_total{};
};

[[nodiscard]] inline Status admit(Bucket& b) noexcept {
    if (b.count >= kBucketLimit) {
        return Status::Busy;
    }
    ++b.count;
    return Status::Ok;
}

[[nodiscard]] inline Status remove(Bucket& b) noexcept {
    if (b.count == 0) {
        return Status::Invalid;
    }
    --b.count;
    return Status::Ok;
}

[[nodiscard]] inline Status validate_rexmt(unsigned rexmt) noexcept {
    if (rexmt > kRexmtLimit) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::tcp_syncache
""",
"net/pbsd.net.tcp_timewait.cppm": """module;
#include <cstdint>

export module pbsd.net.tcp_timewait;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_timer.h — 2MSL timewait bucket sizing.
export namespace pbsd::net::tcp_timewait {

inline constexpr unsigned kBucketCount = 32;
inline constexpr unsigned kMslSeconds = 30;
inline constexpr unsigned k2MslMs = kMslSeconds * 2 * 1000;

struct Bucket {
    unsigned entries{};
};

[[nodiscard]] inline Status validate_bucket(unsigned idx) noexcept {
    if (idx >= kBucketCount) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned bucket_index(std::uint32_t hash) noexcept {
    return hash % kBucketCount;
}

[[nodiscard]] inline Status enter(Bucket& b) noexcept {
    ++b.entries;
    return Status::Ok;
}

[[nodiscard]] inline Status leave(Bucket& b) noexcept {
    if (b.entries == 0) {
        return Status::Invalid;
    }
    --b.entries;
    return Status::Ok;
}

} // namespace pbsd::net::tcp_timewait
""",
"net/pbsd.net.ip_reass.cppm": """module;
#include <cstdint>

export module pbsd.net.ip_reass;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_var.h — IPv4 fragment reassembly limits.
export namespace pbsd::net::ip_reass {

inline constexpr unsigned kMaxQueueLen = 100;
inline constexpr unsigned kMaxFragLen = 65535;
inline constexpr unsigned kMaxLifetimeSec = 30;

struct QueueStats {
    unsigned queued{};
    unsigned timed_out{};
};

[[nodiscard]] inline Status validate_frag(unsigned offset, unsigned len) noexcept {
    if (len == 0 || len > kMaxFragLen) {
        return Status::Invalid;
    }
    if ((offset & 0x7) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status admit(QueueStats& qs) noexcept {
    if (qs.queued >= kMaxQueueLen) {
        return Status::Busy;
    }
    ++qs.queued;
    return Status::Ok;
}

} // namespace pbsd::net::ip_reass
""",
"net/pbsd.net.ip_options.cppm": """module;
#include <cstdint>

export module pbsd.net.ip_options;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_var.h — IPv4 option classes.
export namespace pbsd::net::ip_options {

inline constexpr unsigned kMaxLen = 40;

enum class Class : unsigned char {
    Control = 0,
    Reserved = 1,
    Measurement = 2,
};

enum class Number : unsigned char {
    Eol = 0,
    Nop = 1,
    Lsrr = 131,
    Ssrr = 137,
    RecordRoute = 7,
    Timestamp = 68,
};

[[nodiscard]] inline Status validate_len(unsigned len) noexcept {
    if (len > kMaxLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_number(Number n) noexcept {
    switch (n) {
    case Number::Eol:
    case Number::Nop:
    case Number::Lsrr:
    case Number::Ssrr:
    case Number::RecordRoute:
    case Number::Timestamp:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::net::ip_options
""",
"net/pbsd.net.in_cksum.cppm": """module;
#include <cstdint>

export module pbsd.net.in_cksum;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_cksum.h — Internet checksum helpers.
export namespace pbsd::net::in_cksum {

[[nodiscard]] inline std::uint16_t fold32(std::uint32_t sum) noexcept {
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return static_cast<std::uint16_t>(~sum);
}

[[nodiscard]] inline std::uint16_t add16(std::uint16_t a, std::uint16_t b) noexcept {
    std::uint32_t sum = static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b);
    if (sum > 0xffff) {
        sum = (sum & 0xffff) + 1;
    }
    return static_cast<std::uint16_t>(sum);
}

[[nodiscard]] inline Status validate_pseudo_len(unsigned len) noexcept {
    if (len > 65535) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::in_cksum
""",
"net/pbsd.net.raw_ip.cppm": """module;
#include <cstdint>

export module pbsd.net.raw_ip;

import pbsd.core;
import pbsd.net.sockopt;

/// PROVENANCE: hbsd/src/sys/netinet/raw_ip.c — raw IP socket options.
export namespace pbsd::net::raw_ip {

enum class Option : int {
    HeaderIncl = 0x0002,
    Ttl        = 0x0004,
    Tos        = 0x0005,
    Recvif     = 0x0006,
};

[[nodiscard]] inline Status check_option(CapabilityRights rights, Option opt,
                                         sockopt::OptDir dir) noexcept {
    (void)opt;
    if (dir == sockopt::OptDir::Set && !has_right(rights, CapabilityRights::Write)) {
        return Status::Denied;
    }
    if (dir == sockopt::OptDir::Get && !has_right(rights, CapabilityRights::Read)) {
        return Status::Denied;
    }
    return Status::Ok;
}

} // namespace pbsd::net::raw_ip
""",
"net/pbsd.net.in_mcast.cppm": """module;
#include <cstdint>

export module pbsd.net.in_mcast;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_mcast.h — IPv4 multicast group limits.
export namespace pbsd::net::in_mcast {

inline constexpr unsigned kMaxGroups = 4096;
inline constexpr unsigned kLoopDefault = 1;

struct Group {
    std::uint32_t addr{};
    unsigned short ifindex{};
};

[[nodiscard]] inline Status validate_group(const Group& g) noexcept {
    if (g.addr < 0xe0000000u || g.addr >= 0xf0000000u) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_count(unsigned count) noexcept {
    if (count > kMaxGroups) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::in_mcast
""",
"net/pbsd.net.ip_encap.cppm": """module;
#include <cstdint>

export module pbsd.net.ip_encap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_encap.h — IP encapsulation types.
export namespace pbsd::net::ip_encap {

enum class Type : unsigned char {
    Direct = 0,
    Gre    = 1,
    Mobile = 2,
    Ipsec  = 3,
    Gif    = 4,
};

struct Entry {
    Type type{Type::Direct};
    unsigned short proto{};
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    switch (t) {
    case Type::Direct:
    case Type::Gre:
    case Type::Mobile:
    case Type::Ipsec:
    case Type::Gif:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_entry(const Entry& e) noexcept {
    if (validate_type(e.type) != Status::Ok) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ip_encap
""",
"net/pbsd.net.if_llatbl.cppm": """module;
#include <cstdint>

export module pbsd.net.if_llatbl;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_llatbl.h — link-layer address table states.
export namespace pbsd::net::if_llatbl {

enum class State : signed char {
    Incomplete = 0,
    Reachable  = 1,
    Stale      = 2,
    Delay      = 3,
    Probe      = 4,
};

enum class Flag : unsigned int {
    Static = 0x00000001,
    Proxy  = 0x00000002,
    Pinned = 0x00000004,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::Incomplete:
    case State::Reachable:
    case State::Stale:
    case State::Delay:
    case State::Probe:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline bool is_reachable(State s) noexcept {
    return s == State::Reachable;
}

} // namespace pbsd::net::if_llatbl
""",
"net/pbsd.net.vnet.cppm": """module;
#include <cstdint>

export module pbsd.net.vnet;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/vnet.h — virtual network stack identifiers.
export namespace pbsd::net::vnet {

inline constexpr unsigned kDefaultId = 0;
inline constexpr unsigned kMaxId = 255;

struct Id {
    unsigned value{kDefaultId};
};

[[nodiscard]] inline Status validate_id(unsigned id) noexcept {
    if (id > kMaxId) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate(const Id& id) noexcept {
    return validate_id(id.value);
}

} // namespace pbsd::net::vnet
""",
"net/pbsd.net.radix.cppm": """module;
#include <cstdint>

export module pbsd.net.radix;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/radix.h — radix route tree node flags.
export namespace pbsd::net::radix {

enum class NodeFlag : unsigned char {
    Normal = 0x01,
    Root   = 0x02,
    Active = 0x04,
};

[[nodiscard]] inline bool is_leaf(unsigned char flags) noexcept {
    return (flags & static_cast<unsigned char>(NodeFlag::Normal)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned char flags) noexcept {
    if ((flags & static_cast<unsigned char>(NodeFlag::Root))
        && !(flags & static_cast<unsigned char>(NodeFlag::Normal))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::radix
""",
"net/pbsd.net.ifq.cppm": """module;
#include <cstdint>

export module pbsd.net.ifq;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/ifq.h — interface output queue limits.
export namespace pbsd::net::ifq {

inline constexpr int kDunitNone = -1;
inline constexpr unsigned kDefaultMaxLen = 50;

struct Queue {
    unsigned len{};
    unsigned maxlen{kDefaultMaxLen};
};

[[nodiscard]] inline Status validate_maxlen(unsigned maxlen) noexcept {
    if (maxlen == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status enqueue(Queue& q) noexcept {
    if (q.len >= q.maxlen) {
        return Status::Busy;
    }
    ++q.len;
    return Status::Ok;
}

[[nodiscard]] inline Status dequeue(Queue& q) noexcept {
    if (q.len == 0) {
        return Status::Invalid;
    }
    --q.len;
    return Status::Ok;
}

} // namespace pbsd::net::ifq
""",
"net/pbsd.net.tcp_fastopen.cppm": """module;
#include <cstdint>

export module pbsd.net.tcp_fastopen;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp.h — TCP Fast Open cookie limits.
export namespace pbsd::net::tcp_fastopen {

inline constexpr unsigned kMinCookieLen = 4;
inline constexpr unsigned kMaxCookieLen = 16;
inline constexpr unsigned kPskLen = 16;

struct Cookie {
    unsigned char bytes[kMaxCookieLen]{};
    unsigned len{};
};

[[nodiscard]] inline Status validate_cookie_len(unsigned len) noexcept {
    if (len < kMinCookieLen || len > kMaxCookieLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_cookie(const Cookie& c) noexcept {
    return validate_cookie_len(c.len);
}

} // namespace pbsd::net::tcp_fastopen
""",
"fs/pbsd.fs.vfs_cache.cppm": """module;
#include <cstdint>

export module pbsd.fs.vfs_cache;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/vfs_cache.c — name cache sizing constants.
export namespace pbsd::fs::vfs_cache {

inline constexpr unsigned kDefaultBuckets = 256;
inline constexpr unsigned kMaxBuckets = 65536;

struct Stats {
    unsigned hits{};
    unsigned misses{};
    unsigned purged{};
};

[[nodiscard]] inline Status validate_buckets(unsigned buckets) noexcept {
    if (buckets == 0 || buckets > kMaxBuckets) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline void record_hit(Stats& s) noexcept {
    ++s.hits;
}

[[nodiscard]] inline void record_miss(Stats& s) noexcept {
    ++s.misses;
}

} // namespace pbsd::fs::vfs_cache
""",
"fs/pbsd.fs.vfs_aio.cppm": """module;
#include <cstdint>

export module pbsd.fs.vfs_aio;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/vfs_aio.c — async I/O state machine.
export namespace pbsd::fs::vfs_aio {

enum class State : unsigned char {
    None = 0,
    Queued = 1,
    InProgress = 2,
    Done = 3,
    Cancelled = 4,
};

enum class Flag : unsigned int {
    Read = 0x0001,
    Write = 0x0002,
    Sync = 0x0004,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::None:
    case State::Queued:
    case State::InProgress:
    case State::Done:
    case State::Cancelled:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline bool is_terminal(State s) noexcept {
    return s == State::Done || s == State::Cancelled;
}

} // namespace pbsd::fs::vfs_aio
""",
"fs/pbsd.fs.ufs_dir.cppm": """module;
#include <cstdint>

export module pbsd.fs.ufs_dir;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/ufs/ufs/dir.h — UFS directory entry layout.
export namespace pbsd::fs::ufs_dir {

inline constexpr unsigned kDirectEntries = 12;
inline constexpr unsigned kNameMax = 255;
inline constexpr unsigned kRecLenMin = 8;

struct Entry {
    unsigned ino{};
    unsigned short reclen{};
    unsigned char type{};
    unsigned char namelen{};
};

[[nodiscard]] inline Status validate_entry(const Entry& e) noexcept {
    if (e.ino == 0 || e.reclen < kRecLenMin) {
        return Status::Invalid;
    }
    if (e.namelen == 0 || e.namelen > kNameMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::ufs_dir
""",
"fs/pbsd.fs.tmpfs_node.cppm": """module;
#include <cstdint>

export module pbsd.fs.tmpfs_node;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/tmpfs/tmpfs.h — tmpfs vnode node types.
export namespace pbsd::fs::tmpfs_node {

enum class Type : unsigned char {
    Dir = 1,
    File = 2,
    Link = 3,
    Whiteout = 4,
};

struct Node {
    Type type{Type::File};
    unsigned short mode{};
    unsigned long long size{};
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    switch (t) {
    case Type::Dir:
    case Type::File:
    case Type::Link:
    case Type::Whiteout:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_node(const Node& n) noexcept {
    if (validate_type(n.type) != Status::Ok) {
        return Status::Invalid;
    }
    if ((n.mode & ~07777u) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::tmpfs_node
""",
"fs/pbsd.fs.null_vnode.cppm": """module;
#include <cstdint>

export module pbsd.fs.null_vnode;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/nullfs/null.h — nullfs bypass flags.
export namespace pbsd::fs::null_vnode {

enum class BypassFlag : unsigned int {
    Read  = 0x0001,
    Write = 0x0002,
    Lookup = 0x0004,
};

struct Target {
    unsigned lower_mount_id{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_target(const Target& t) noexcept {
    if (t.lower_mount_id == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool bypass_read(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(BypassFlag::Read)) != 0;
}

} // namespace pbsd::fs::null_vnode
""",
"geom/pbsd.geom.linux_lvm.cppm": """module;
#include <cstdint>

export module pbsd.geom.linux_lvm;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/linux_lvm/g_linux_lvm.h — Linux LVM label constants.
export namespace pbsd::geom::linux_lvm {

inline constexpr unsigned kNameLen = 128;
inline constexpr unsigned kUuidLen = 40;

struct Label {
    std::uint64_t sector{};
    std::uint32_t crc{};
    char uuid[kUuidLen]{};
    std::uint64_t size{};
};

[[nodiscard]] inline Status validate_label(const Label& lbl) noexcept {
    if (lbl.sector == 0 || lbl.size == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::linux_lvm
""",
"geom/pbsd.geom.subr.cppm": """module;
#include <cstdint>

export module pbsd.geom.subr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_subr.c — GEOM topology helper flags.
export namespace pbsd::geom::subr {

enum class TopoFlag : unsigned int {
    Orphan = 0x0001,
    Wither = 0x0002,
    Hardcoded = 0x0004,
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    (void)flags;
    return Status::Ok;
}

[[nodiscard]] inline bool is_wither(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(TopoFlag::Wither)) != 0;
}

} // namespace pbsd::geom::subr
""",
"geom/pbsd.geom.io.cppm": """module;
#include <cstdint>

export module pbsd.geom.io;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_io.c — GEOM I/O request directions.
export namespace pbsd::geom::io {

enum class Direction : unsigned char {
    Read = 0,
    Write = 1,
    Flush = 2,
    Delete = 3,
};

struct Request {
    Direction dir{Direction::Read};
    std::uint64_t offset{};
    unsigned length{};
};

[[nodiscard]] inline Status validate_request(const Request& r) noexcept {
    if (r.length == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::io
""",
"geom/pbsd.geom.event.cppm": """module;
#include <cstdint>

export module pbsd.geom.event;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_event.c — GEOM event notification types.
export namespace pbsd::geom::event {

enum class Type : unsigned char {
    ProviderAdded = 0,
    ProviderRemoved = 1,
    ConsumerAdded = 2,
    ConsumerRemoved = 3,
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    switch (t) {
    case Type::ProviderAdded:
    case Type::ProviderRemoved:
    case Type::ConsumerAdded:
    case Type::ConsumerRemoved:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::geom::event
""",
"geom/pbsd.geom.vfs.cppm": """module;
#include <cstdint>

export module pbsd.geom.vfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_vfs.h — GEOM VFS mount hooks.
export namespace pbsd::geom::vfs {

enum class Hook : unsigned char {
    Mount = 0,
    Unmount = 1,
    Sync = 2,
};

[[nodiscard]] inline Status validate_hook(Hook h) noexcept {
    switch (h) {
    case Hook::Mount:
    case Hook::Unmount:
    case Hook::Sync:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::geom::vfs
""",
"zfs/pbsd.zfs.txg.cppm": """module;
#include <cstdint>

export module pbsd.zfs.txg;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/txg.h — transaction groups.
export namespace pbsd::zfs::txg {

inline constexpr unsigned kConcurrentStates = 3;
inline constexpr unsigned kSize = 4;
inline constexpr unsigned kMask = kSize - 1;
inline constexpr unsigned kInitial = kSize;
inline constexpr unsigned kDeferSize = 2;

[[nodiscard]] inline unsigned idx(std::uint64_t txg) noexcept {
    return static_cast<unsigned>(txg & kMask);
}

[[nodiscard]] inline Status validate_txg(std::uint64_t txg) noexcept {
    if (txg == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::txg
""",
"zfs/pbsd.zfs.uberblock.cppm": """module;
#include <cstdint>

export module pbsd.zfs.uberblock;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/uberblock_impl.h — uberblock magic.
export namespace pbsd::zfs::uberblock {

inline constexpr std::uint64_t kMagic = 0x00bab10cULL;
inline constexpr unsigned kShift = 10;

struct Header {
    std::uint64_t magic{kMagic};
    std::uint64_t txg{};
    std::uint64_t guid{};
};

[[nodiscard]] inline Status validate_magic(std::uint64_t magic) noexcept {
    return magic == kMagic ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_header(const Header& h) noexcept {
    if (validate_magic(h.magic) != Status::Ok || h.txg == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::uberblock
""",
"zfs/pbsd.zfs.space_map.cppm": """module;
#include <cstdint>

export module pbsd.zfs.space_map;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/space_map.h — space map sizing.
export namespace pbsd::zfs::space_map {

inline constexpr unsigned kHistogramSize = 32;
inline constexpr unsigned kSizeV0Fields = 3;

struct Phys {
    std::uint64_t length{};
    std::int64_t alloc{};
};

[[nodiscard]] inline Status validate_phys(const Phys& p) noexcept {
    if (p.length == 0) {
        return Status::Invalid;
    }
    if (p.alloc < 0 || static_cast<std::uint64_t>(p.alloc) > p.length) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t free_bytes(const Phys& p) noexcept {
    if (validate_phys(p) != Status::Ok) {
        return 0;
    }
    return p.length - static_cast<std::uint64_t>(p.alloc);
}

} // namespace pbsd::zfs::space_map
""",
"zfs/pbsd.zfs.vdev_queue.cppm": """module;
#include <cstdint>

export module pbsd.zfs.vdev_queue;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/vdev_queue.h — vdev I/O queue classes.
export namespace pbsd::zfs::vdev_queue {

enum class Priority : unsigned char {
    SyncRead = 0,
    SyncWrite = 1,
    AsyncRead = 2,
    AsyncWrite = 3,
    Scrub = 4,
};

struct QueueStats {
    unsigned pending{};
    unsigned active{};
    unsigned max_active{64};
};

[[nodiscard]] inline Status admit(QueueStats& q) noexcept {
    if (q.active >= q.max_active) {
        return Status::Busy;
    }
    ++q.pending;
    return Status::Ok;
}

[[nodiscard]] inline Status dispatch(QueueStats& q) noexcept {
    if (q.pending == 0) {
        return Status::Invalid;
    }
    --q.pending;
    ++q.active;
    return Status::Ok;
}

} // namespace pbsd::zfs::vdev_queue
""",
"zfs/pbsd.zfs.spa_load.cppm": """module;
#include <cstdint>

export module pbsd.zfs.spa_load;

import pbsd.core;
import pbsd.zfs.spa;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/spa.h — pool import/load policy.
export namespace pbsd::zfs::spa_load {

enum class Policy : unsigned char {
    Normal = 0,
    ImportOnly = 1,
    TryImport = 2,
    NoRewind = 3,
};

[[nodiscard]] inline Status validate_policy(Policy p) noexcept {
    switch (p) {
    case Policy::Normal:
    case Policy::ImportOnly:
    case Policy::TryImport:
    case Policy::NoRewind:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status can_load(spa::State state, Policy policy) noexcept {
    if (spa::validate_state(state) != Status::Ok) {
        return Status::Invalid;
    }
    if (policy == Policy::ImportOnly && state == spa::State::Active) {
        return Status::Busy;
    }
    return spa::is_importable(state) ? Status::Ok : Status::Denied;
}

} // namespace pbsd::zfs::spa_load
""",
}


def main() -> None:
    created = 0
    for rel, content in MODULES.items():
        path = ROOT / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        if not path.exists() or path.read_text(encoding="utf-8") != content:
            path.write_text(content, encoding="utf-8", newline="\n")
            created += 1
    print(f"Wrote/updated {created} of {len(MODULES)} module files")


if __name__ == "__main__":
    main()
