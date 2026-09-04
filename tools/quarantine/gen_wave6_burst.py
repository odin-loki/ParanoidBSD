#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate Wave 6 net/fs/geom/zfs module files."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd"

MODULES = {
"net/pbsd.net.lagg.cppm": """module;
#include <cstdint>

export module pbsd.net.lagg;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_lagg.h — LAGG protocol and port flags.
export namespace pbsd::net::lagg {

inline constexpr unsigned kMaxPorts = 32;
inline constexpr unsigned kMaxStacking = 4;

enum class Flag : unsigned int {
    HashL2 = 0x00000001,
    HashL3 = 0x00000002,
    HashL4 = 0x00000004,
};

enum class PortFlag : unsigned int {
    Slave        = 0x00000000,
    Master       = 0x00000001,
    Stack        = 0x00000002,
    Active       = 0x00000004,
    Collecting   = 0x00000008,
    Distributing = 0x00000010,
};

enum class Proto : unsigned char {
    None = 0,
    RoundRobin,
    Failover,
    LoadBalance,
    Lacp,
    Broadcast,
};

[[nodiscard]] inline Status validate_proto(Proto p) noexcept {
    if (p == Proto::None) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_port_count(unsigned count) noexcept {
    if (count == 0 || count > kMaxPorts) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::lagg
""",
"net/pbsd.net.gif.cppm": """module;
#include <cstdint>

export module pbsd.net.gif;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_gif.h — GIF tunnel MTU and option flags.
export namespace pbsd::net::gif {

inline constexpr unsigned kMtuDefault = 1280;
inline constexpr unsigned kMtuMin = 1280;
inline constexpr unsigned kMtuMax = 8192;
inline constexpr unsigned kEtheripVersion = 0x3;

enum class Opt : unsigned int {
    NoClamp       = 0x0001,
    IgnoreSource  = 0x0002,
};

[[nodiscard]] inline Status validate_mtu(unsigned mtu) noexcept {
    if (mtu < kMtuMin || mtu > kMtuMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool opt_has(unsigned o, Opt bit) noexcept {
    return (o & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::net::gif
""",
"net/pbsd.net.tun.cppm": """module;
#include <cstdint>

export module pbsd.net.tun;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_tun.h — TUN device MTU and ioctls.
export namespace pbsd::net::tun {

inline constexpr unsigned kMtuDefault = 1500;
inline constexpr unsigned kMruMax = 65535;

struct Info {
    int baudrate{};
    unsigned short mtu{};
    unsigned char type{};
};

[[nodiscard]] inline Status validate_mtu(unsigned mtu) noexcept {
    if (mtu == 0 || mtu > kMruMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_info(Info const& info) noexcept {
    return validate_mtu(info.mtu);
}

} // namespace pbsd::net::tun
""",
"net/pbsd.net.tap.cppm": """module;
#include <cstdint>

export module pbsd.net.tap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_tap.h — TAP bridge device constants.
export namespace pbsd::net::tap {

inline constexpr unsigned kMruMax = 65535;

enum class VnetHdr : unsigned char {
    Disabled = 0,
    Enabled  = 1,
};

[[nodiscard]] inline Status validate_mru(unsigned mru) noexcept {
    if (mru == 0 || mru > kMruMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::tap
""",
"net/pbsd.net.loop.cppm": """module;
#include <cstdint>

export module pbsd.net.loop;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if.h — loopback interface flags.
export namespace pbsd::net::loopback {

inline constexpr unsigned kIffLoopback = 0x8;
inline constexpr unsigned kMtuDefault = 16384;

enum class Flag : unsigned int {
    Up       = 0x1,
    Loopback = 0x8,
    Running  = 0x40,
};

[[nodiscard]] inline constexpr bool is_loopback(unsigned flags) noexcept {
    return (flags & kIffLoopback) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (!is_loopback(flags)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::loopback
""",
"net/pbsd.net.ipfw.cppm": """module;
#include <cstdint>

export module pbsd.net.ipfw;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_fw.h — ipfw(4) rule and set limits.
export namespace pbsd::net::ipfw {

inline constexpr unsigned kDefaultRule = 65535;
inline constexpr unsigned kMaxSets = 32;
inline constexpr unsigned kResvdSet = 31;
inline constexpr unsigned kArgMin = 1;
inline constexpr unsigned kArgMax = 65534;
inline constexpr unsigned kCallstackSize = 16;

enum class OpVer : unsigned short {
    V0 = 0,
    V1 = 1,
};

enum class ArgFlag : unsigned int {
    Ether  = 0x00010000,
    Nh4    = 0x00020000,
    Nh6    = 0x00040000,
    In     = 0x00400000,
    Out    = 0x00800000,
    Ip4    = 0x01000000,
    Ip6    = 0x02000000,
};

[[nodiscard]] inline Status validate_rule_num(unsigned rule) noexcept {
    if (rule > kDefaultRule) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_set(unsigned set) noexcept {
    if (set >= kMaxSets) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ipfw
""",
"net/pbsd.net.carp.cppm": """module;
#include <cstdint>

export module pbsd.net.carp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_carp.h — CARP/VRRP advertisement constants.
export namespace pbsd::net::carp {

inline constexpr unsigned char kDefaultTtl = 255;
inline constexpr unsigned char kDefaultInterval = 1;
inline constexpr unsigned char kAdvertisement = 0x01;
inline constexpr unsigned kKeyLen = 20;
inline constexpr unsigned kVrrpMaxInterval = 0x1000 - 1;

struct Header {
    unsigned char version_type{};
    unsigned char vhid{};
    unsigned char advskew{};
    unsigned char authlen{};
    unsigned char advbase{};
    unsigned short cksum{};
};

[[nodiscard]] inline Status validate_header(Header const& h) noexcept {
    if (h.vhid == 0) {
        return Status::Invalid;
    }
    if (h.advbase == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::carp
""",
"net/pbsd.net.ndp.cppm": """module;
#include <cstdint>

export module pbsd.net.ndp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/nd6.h — ND6 neighbor cache states.
export namespace pbsd::net::ndp {

enum class LlState : signed char {
    NoState    = -2,
    Incomplete = 0,
    Reachable  = 1,
    Stale      = 2,
    Delay      = 3,
    Probe      = 4,
};

enum class Flag : unsigned int {
    PerformNud = 0x00000001,
    AcceptRtadv = 0x00000002,
    PreferIflladdr = 0x00000004,
};

[[nodiscard]] inline Status validate_state(LlState s) noexcept {
    switch (s) {
    case LlState::Incomplete:
    case LlState::Reachable:
    case LlState::Stale:
    case LlState::Delay:
    case LlState::Probe:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline constexpr bool is_probreach(LlState s) noexcept {
    return static_cast<signed char>(s) > static_cast<signed char>(LlState::Incomplete);
}

} // namespace pbsd::net::ndp
""",
"net/pbsd.net.ip6.cppm": """module;
#include <cstdint>

export module pbsd.net.ip6;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip6.h — IPv6 header and next-header constants.
export namespace pbsd::net::ip6 {

inline constexpr unsigned char kVersion = 6;
inline constexpr unsigned kAddrBytes = 16;
inline constexpr unsigned char kHopLimitDefault = 64;

enum class NextHeader : unsigned char {
    HopByHop = 0,
    Icmp6    = 58,
    None     = 59,
    DestOpts = 60,
    Routing  = 43,
    Fragment = 44,
    Udp      = 17,
    Tcp      = 6,
};

struct Header {
    unsigned char vfc{};
    unsigned char flow[3]{};
    unsigned short payload_len{};
    unsigned char nxt{};
    unsigned char hlim{};
};

[[nodiscard]] inline Status validate_header(Header const& h) noexcept {
    unsigned ver = (h.vfc >> 4) & 0x0F;
    if (ver != kVersion) {
        return Status::Protocol;
    }
    if (h.hlim == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ip6
""",
"net/pbsd.net.udp6.cppm": """module;
#include <cstdint>

export module pbsd.net.udp6;

import pbsd.core;
import pbsd.net.sockopt;

/// PROVENANCE: hbsd/src/sys/netinet/in.h, udp.h — IPv6 UDP socket options.
export namespace pbsd::net::udp6 {

enum class Option : int {
    Checksum = 0x0001,
    Encap    = 0x0002,
    Pktinfo  = 0x0003,
    Recvpktinfo = 0x0004,
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

} // namespace pbsd::net::udp6
""",
"net/pbsd.net.tcp6.cppm": """module;
#include <cstdint>

export module pbsd.net.tcp6;

import pbsd.core;
import pbsd.net.sockopt;

/// PROVENANCE: hbsd/src/sys/netinet/tcp.h, tcp_fsm.h — IPv6 TCP socket options.
export namespace pbsd::net::tcp6 {

enum class Option : int {
    NoDelay   = 0x0001,
    MaxSeg    = 0x0002,
    KeepIdle  = 0x0003,
    KeepIntvl = 0x0004,
    KeepCnt   = 0x0005,
    Pktinfo   = 0x0006,
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

} // namespace pbsd::net::tcp6
""",
"net/pbsd.net.sctp.cppm": """module;
#include <cstdint>

export module pbsd.net.sctp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/sctp.h — SCTP chunk types and socket options.
export namespace pbsd::net::sctp {

enum class ChunkType : unsigned char {
    Data        = 0,
    Init        = 1,
    InitAck     = 2,
    Sack        = 3,
    Heartbeat   = 4,
    HeartbeatAck = 5,
    Abort       = 6,
    Shutdown    = 7,
    ShutdownAck = 8,
};

enum class Option : unsigned int {
    RtoInfo  = 0x00000001,
    AssocInfo = 0x00000002,
    InitMsg  = 0x00000003,
    NoDelay  = 0x00000004,
};

struct Header {
    unsigned short src_port{};
    unsigned short dest_port{};
    unsigned v_tag{};
    unsigned checksum{};
};

[[nodiscard]] inline Status validate_header(Header const& h) noexcept {
    if (h.src_port == 0 || h.dest_port == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_chunk(ChunkType t) noexcept {
    switch (t) {
    case ChunkType::Data:
    case ChunkType::Init:
    case ChunkType::InitAck:
    case ChunkType::Sack:
    case ChunkType::Abort:
    case ChunkType::Shutdown:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::net::sctp
""",
"net/pbsd.net.netmap.cppm": """module;
#include <cstdint>

export module pbsd.net.netmap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/netmap.h — netmap API version and ring flags.
export namespace pbsd::net::netmap {

inline constexpr unsigned kApi = 14;
inline constexpr unsigned kMinApi = 14;
inline constexpr unsigned kMaxApi = 15;
inline constexpr unsigned kCacheAlign = 128;

enum class RingFlag : unsigned int {
    TxRing = 0x0001,
    RxRing = 0x0002,
    HostRing = 0x0004,
};

[[nodiscard]] inline Status validate_api(unsigned api) noexcept {
    if (api < kMinApi || api > kMaxApi) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::netmap
""",
"net/pbsd.net.netgraph.cppm": """module;
#include <cstdint>

export module pbsd.net.netgraph;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netgraph/ng_message.h — netgraph message limits.
export namespace pbsd::net::netgraph {

inline constexpr unsigned kTypeSize = 32;
inline constexpr unsigned kHookSize = 32;
inline constexpr unsigned kNodeSize = 32;
inline constexpr unsigned kPathSize = 512;
inline constexpr unsigned kCmdStrSize = 32;
inline constexpr unsigned kTextResponse = 1024;
inline constexpr unsigned kAbiVersion = 12;

enum class MsgType : unsigned char {
    Connect = 1,
    Disconnect = 2,
    Generic = 3,
    TextStatus = 4,
};

[[nodiscard]] inline Status validate_name_len(unsigned len, unsigned max) noexcept {
    if (len == 0 || len >= max) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::netgraph
""",
"fs/pbsd.fs.nfs.cppm": """module;
#include <cstdint>

export module pbsd.fs.nfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/nfs/nfsproto.h — NFS protocol constants.
export namespace pbsd::fs::nfs {

inline constexpr unsigned kPort = 2049;
inline constexpr unsigned kProg = 100003;
inline constexpr unsigned kVer2 = 2;
inline constexpr unsigned kVer3 = 3;
inline constexpr unsigned kVer4 = 4;
inline constexpr unsigned kMaxPathLen = 1024;
inline constexpr unsigned kMaxNameLen = 255;

enum class Proc : unsigned short {
    Null = 0,
    GetAttr = 1,
    Lookup = 3,
    Read = 6,
    Write = 8,
};

[[nodiscard]] inline Status validate_version(unsigned ver) noexcept {
    if (ver < kVer2 || ver > kVer4) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::nfs
""",
"fs/pbsd.fs.nullfs.cppm": """module;
#include <cstdint>

export module pbsd.fs.nullfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/nullfs/null.h — nullfs mount flags.
export namespace pbsd::fs::nullfs {

enum class MountFlag : unsigned long long {
    Cache       = 0x0001,
    NoUnpbypass = 0x0002,
};

[[nodiscard]] inline Status validate_flags(unsigned long long flags) noexcept {
    (void)flags;
    return Status::Ok;
}

} // namespace pbsd::fs::nullfs
""",
"fs/pbsd.fs.procfs.cppm": """module;
#include <cstdint>

export module pbsd.fs.procfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/procfs/procfs.h — procfs node types.
export namespace pbsd::fs::procfs {

enum class NodeType : unsigned char {
    Root = 0,
    Curproc = 1,
    File = 2,
    Mem = 3,
    Regs = 4,
    Fpregs = 5,
};

[[nodiscard]] inline Status validate_node(NodeType t) noexcept {
    switch (t) {
    case NodeType::Root:
    case NodeType::Curproc:
    case NodeType::File:
    case NodeType::Mem:
    case NodeType::Regs:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::procfs
""",
"fs/pbsd.fs.devfs.cppm": """module;
#include <cstdint>

export module pbsd.fs.devfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/devfs/devfs.h — devfs magic and rule identifiers.
export namespace pbsd::fs::devfs {

inline constexpr unsigned kMagic = 0xdb0a087a;

enum class RuleAction : unsigned char {
    Hide = 0,
    Show = 1,
    Link = 2,
};

[[nodiscard]] inline Status validate_rule_id(unsigned id) noexcept {
    if (id == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::devfs
""",
"fs/pbsd.fs.fdescfs.cppm": """module;
#include <cstdint>

export module pbsd.fs.fdescfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/fdescfs/fdesc.h — fdescfs mount and node ids.
export namespace pbsd::fs::fdescfs {

enum class MountFlag : int {
    Unmountf = 0x01,
    Linrdlnkf = 0x02,
    Nodup = 0x04,
    Rdlnkf = 0x08,
};

enum class NodeId : int {
    Root = 1,
    Desc = 3,
};

[[nodiscard]] inline Status validate_node(NodeId id) noexcept {
    switch (id) {
    case NodeId::Root:
    case NodeId::Desc:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::fdescfs
""",
"fs/pbsd.fs.linprocfs.cppm": """module;
#include <cstdint>

export module pbsd.fs.linprocfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/compat/linprocfs/linprocfs.c — Linux /proc compatibility nodes.
export namespace pbsd::fs::linprocfs {

inline constexpr unsigned kNameLen = 128;

enum class Node : unsigned char {
    Root = 0,
    Self = 1,
    Cpuinfo = 2,
    Meminfo = 3,
    Mounts = 4,
    Stat = 5,
};

[[nodiscard]] inline Status validate_node(Node n) noexcept {
    switch (n) {
    case Node::Root:
    case Node::Self:
    case Node::Cpuinfo:
    case Node::Meminfo:
    case Node::Mounts:
    case Node::Stat:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::linprocfs
""",
"fs/pbsd.fs.fusefs.cppm": """module;
#include <cstdint>

export module pbsd.fs.fusefs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/fuse/fuse.h — FUSE protocol version and init flags.
export namespace pbsd::fs::fusefs {

inline constexpr unsigned kMajorVersion = 7;
inline constexpr unsigned kMinorVersion = 31;

enum class InitFlag : unsigned int {
    AsyncRead   = 1u << 0,
    PosixLocks  = 1u << 1,
    ExportSupport = 1u << 4,
    DontMask    = 1u << 6,
};

[[nodiscard]] inline Status validate_version(unsigned major, unsigned minor) noexcept {
    if (major != kMajorVersion || minor > kMinorVersion) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::fusefs
""",
"geom/pbsd.geom.raid3.cppm": """module;
#include <cstdint>

export module pbsd.geom.raid3;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/raid3/g_raid3.h — GEOM RAID3 metadata version.
export namespace pbsd::geom::raid3 {

inline constexpr unsigned kVersion = 5;

enum class DiskFlag : unsigned long long {
    Dirty         = 0x0000000000000001ULL,
    Synchronizing = 0x0000000000000002ULL,
};

[[nodiscard]] inline Status validate_width(unsigned width) noexcept {
    if (width < 3) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::raid3
""",
"geom/pbsd.geom.eli.cppm": """module;
#include <cstdint>

export module pbsd.geom.eli;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/eli/g_eli.h — GEOM ELI encryption flags.
export namespace pbsd::geom::eli {

inline constexpr unsigned kVersion = 7;

enum class Algo : unsigned char {
    None = 0,
    AesXts = 1,
    AesCbc = 2,
    CamelliaCbc = 3,
};

enum class Flag : unsigned int {
    Boot    = 0x00000001,
    Geli    = 0x00000002,
    Onetime = 0x00000004,
};

[[nodiscard]] inline Status validate_algo(Algo a) noexcept {
    if (a == Algo::None) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::eli
""",
"geom/pbsd.geom.journal.cppm": """module;
#include <cstdint>

export module pbsd.geom.journal;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/journal/g_journal.h — GEOM JOURNAL metadata.
export namespace pbsd::geom::journal {

inline constexpr unsigned kVersion = 0;

enum class State : unsigned char {
    New = 0,
    Active = 1,
    Dirty = 2,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::New:
    case State::Active:
    case State::Dirty:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::geom::journal
""",
"geom/pbsd.geom.gate.cppm": """module;
#include <cstdint>

export module pbsd.geom.gate;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/gate/g_gate.h — GEOM GATE provider constants.
export namespace pbsd::geom::gate {

inline constexpr unsigned kVersion = 3;
inline constexpr unsigned kMaxQueueSize = 4096;

enum class Cmd : unsigned char {
    Create = 0,
    Destroy = 1,
    Start = 2,
    Stop = 3,
};

[[nodiscard]] inline Status validate_cmd(Cmd c) noexcept {
    switch (c) {
    case Cmd::Create:
    case Cmd::Destroy:
    case Cmd::Start:
    case Cmd::Stop:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::geom::gate
""",
"geom/pbsd.geom.multipath.cppm": """module;
#include <cstdint>

export module pbsd.geom.multipath;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/multipath/g_multipath.h — GEOM MULTIPATH metadata.
export namespace pbsd::geom::multipath {

inline constexpr unsigned kVersion = 1;

enum class State : unsigned char {
    New = 0,
    Active = 1,
    Failed = 2,
};

[[nodiscard]] inline Status validate_provider_count(unsigned count) noexcept {
    if (count < 2) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::New:
    case State::Active:
    case State::Failed:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::geom::multipath
""",
"zfs/pbsd.zfs.zio_checksum.cppm": """module;
#include <cstdint>

export module pbsd.zfs.zio_checksum;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zio_checksum.h — checksum algorithms.
export namespace pbsd::zfs::zio_checksum {

enum class Algorithm : unsigned char {
    Inherit = 0,
    On,
    Off,
    Label,
    GangHeader,
    Zilog,
    Fletcher2,
    Fletcher4,
    Sha256,
    Zilog2,
    NoParity,
    Sha512,
    Skein,
    Edonr,
    Blake3,
    Functions,
};

enum class Flag : unsigned int {
    Metadata = 1u << 1,
    Embedded = 1u << 2,
    Dedup    = 1u << 3,
    Salted   = 1u << 4,
    Nopwrite = 1u << 5,
};

struct Info {
    Algorithm algo{};
    Flag flags{};
    const char* name{};
};

inline constexpr Info kTable[] = {
    {Algorithm::Fletcher4, Flag::Metadata, "fletcher4"},
    {Algorithm::Sha256, Flag::Metadata | Flag::Salted, "sha256"},
    {Algorithm::Sha512, Flag::Metadata | Flag::Salted, "sha512"},
    {Algorithm::Skein, Flag::Metadata | Flag::Salted, "skein"},
    {Algorithm::Blake3, Flag::Metadata | Flag::Salted, "blake3"},
    {Algorithm::Off, static_cast<Flag>(0), "off"},
    {Algorithm::On, Flag::Metadata, "on"},
};

[[nodiscard]] inline constexpr std::size_t table_size() noexcept {
    return sizeof(kTable) / sizeof(kTable[0]);
}

[[nodiscard]] inline Status validate_algo(Algorithm a) noexcept {
    if (a >= Algorithm::Functions) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool flag_has(Flag f, Flag bit) noexcept {
    return (static_cast<unsigned>(f) & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::zfs::zio_checksum
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
