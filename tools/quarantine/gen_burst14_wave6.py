#!/usr/bin/env python3
"""Burst 14 — net/fs/geom/zfs gap-fill hand port modules."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd"

MODULES = {
"net/pbsd.net.tcp_input.cppm": """module;
#include <cstdint>

export module pbsd.net.tcp_input;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_input.c — TCP input processing flags.
export namespace pbsd::net::tcp_input {

enum class Flag : unsigned {
    AckNow   = 0x0001,
    Drop     = 0x0002,
    Deliver  = 0x0004,
    Sack     = 0x0008,
    FastPath = 0x0010,
};

struct Stats {
    unsigned segments{};
    unsigned drops{};
    unsigned fast_path{};
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & ~(static_cast<unsigned>(Flag::AckNow) | static_cast<unsigned>(Flag::Drop)
                   | static_cast<unsigned>(Flag::Deliver) | static_cast<unsigned>(Flag::Sack)
                   | static_cast<unsigned>(Flag::FastPath))) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status account(Stats& st, unsigned flags) noexcept {
    if (validate_flags(flags) != Status::Ok) {
        return Status::Invalid;
    }
    ++st.segments;
    if ((flags & static_cast<unsigned>(Flag::Drop)) != 0) {
        ++st.drops;
        return Status::Protocol;
    }
    if ((flags & static_cast<unsigned>(Flag::FastPath)) != 0) {
        ++st.fast_path;
    }
    return Status::Ok;
}

} // namespace pbsd::net::tcp_input
""",
"net/pbsd.net.tcp_output.cppm": """module;
#include <cstdint>

export module pbsd.net.tcp_output;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_output.c — TCP output segment sizing.
export namespace pbsd::net::tcp_output {

inline constexpr unsigned kMinMss = 536;
inline constexpr unsigned kMaxMss = 65535;
inline constexpr unsigned kDefaultMss = 1460;

struct Segment {
    unsigned seq{};
    unsigned len{};
    unsigned mss{kDefaultMss};
    bool push{false};
};

[[nodiscard]] inline Status validate_mss(unsigned mss) noexcept {
    if (mss < kMinMss || mss > kMaxMss) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_segment(const Segment& seg) noexcept {
    if (validate_mss(seg.mss) != Status::Ok) {
        return Status::Invalid;
    }
    if (seg.len > seg.mss) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned payload_room(unsigned mss, unsigned hdr_len) noexcept {
    if (mss <= hdr_len) {
        return 0;
    }
    return mss - hdr_len;
}

} // namespace pbsd::net::tcp_output
""",
"net/pbsd.net.in_pcbgroup.cppm": """module;
#include <cstdint>

export module pbsd.net.in_pcbgroup;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_pcbgroup.h — PCB hash bucket groups.
export namespace pbsd::net::in_pcbgroup {

inline constexpr unsigned kDefaultGroups = 16;
inline constexpr unsigned kMaxGroups = 256;

struct Group {
    unsigned count{};
    unsigned max{kDefaultGroups};
};

[[nodiscard]] inline Status validate_group_count(unsigned groups) noexcept {
    if (groups == 0 || groups > kMaxGroups) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned bucket_index(std::uint32_t hash, unsigned groups) noexcept {
    if (validate_group_count(groups) != Status::Ok) {
        return 0;
    }
    return hash % groups;
}

[[nodiscard]] inline Status admit(Group& g) noexcept {
    if (g.count >= g.max) {
        return Status::Busy;
    }
    ++g.count;
    return Status::Ok;
}

} // namespace pbsd::net::in_pcbgroup
""",
"net/pbsd.net.in_proto.cppm": """module;
#include <cstdint>

export module pbsd.net.in_proto;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_proto.c — IPv4 protocol switch entries.
export namespace pbsd::net::in_proto {

enum class Proto : unsigned char {
    Icmp = 1,
    Igmp = 2,
    Tcp  = 6,
    Udp  = 17,
    Raw  = 255,
};

struct Entry {
    Proto proto{Proto::Tcp};
    bool reachable{true};
};

[[nodiscard]] inline Status validate_proto(Proto p) noexcept {
    switch (p) {
    case Proto::Icmp:
    case Proto::Igmp:
    case Proto::Tcp:
    case Proto::Udp:
    case Proto::Raw:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status dispatch(const Entry& e) noexcept {
    if (!e.reachable) {
        return Status::Denied;
    }
    return validate_proto(e.proto);
}

} // namespace pbsd::net::in_proto
""",
"net/pbsd.net.ip_fw_nat.cppm": """module;
#include <cstdint>

export module pbsd.net.ip_fw_nat;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netpfil/ipfw/ip_fw_nat.h — ipfw NAT rule flags.
export namespace pbsd::net::ip_fw_nat {

enum class Flag : unsigned {
    In  = 0x0001,
    Out = 0x0002,
    Any = 0x0004,
};

struct Rule {
    unsigned id{};
    unsigned flags{};
    std::uint32_t addr{};
    unsigned short port{};
};

[[nodiscard]] inline Status validate_rule(const Rule& r) noexcept {
    if (r.id == 0) {
        return Status::Invalid;
    }
    if ((r.flags & ~(static_cast<unsigned>(Flag::In) | static_cast<unsigned>(Flag::Out)
                     | static_cast<unsigned>(Flag::Any))) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool matches_direction(unsigned flags, bool outbound) noexcept {
    if ((flags & static_cast<unsigned>(Flag::Any)) != 0) {
        return true;
    }
    if (outbound) {
        return (flags & static_cast<unsigned>(Flag::Out)) != 0;
    }
    return (flags & static_cast<unsigned>(Flag::In)) != 0;
}

} // namespace pbsd::net::ip_fw_nat
""",
"net/pbsd.net.ether_ifattach.cppm": """module;
#include <cstdint>

export module pbsd.net.ether_ifattach;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_ethersubr.c — Ethernet ifattach flags.
export namespace pbsd::net::ether_ifattach {

enum class Flag : unsigned {
    Broadcast = 0x0001,
    Multicast = 0x0002,
    Promisc   = 0x0004,
};

struct Attach {
    unsigned short ifindex{};
    unsigned flags{};
    unsigned char mac[6]{};
};

[[nodiscard]] inline Status validate_mac(const unsigned char mac[6]) noexcept {
    bool all_zero = true;
    for (unsigned i = 0; i < 6; ++i) {
        if (mac[i] != 0) {
            all_zero = false;
            break;
        }
    }
    return all_zero ? Status::Invalid : Status::Ok;
}

[[nodiscard]] inline Status validate_attach(const Attach& a) noexcept {
    if (a.ifindex == 0) {
        return Status::Invalid;
    }
    return validate_mac(a.mac);
}

} // namespace pbsd::net::ether_ifattach
""",
"fs/pbsd.fs.fifofs.cppm": """module;

export module pbsd.fs.fifofs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/fifofs — FIFO vnode operations scaffold.
export namespace pbsd::fs::fifofs {

enum class Op : unsigned char {
    Open = 0,
    Read = 1,
    Write = 2,
    Poll = 3,
    Close = 4,
};

[[nodiscard]] inline Status validate_op(Op op) noexcept {
    switch (op) {
    case Op::Open:
    case Op::Read:
    case Op::Write:
    case Op::Poll:
    case Op::Close:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline bool is_reader_blocked(unsigned readers, unsigned buffer_free) noexcept {
    return readers > 0 && buffer_free == 0;
}

} // namespace pbsd::fs::fifofs
""",
"fs/pbsd.fs.ufs_inode.cppm": """module;
#include <cstdint>

export module pbsd.fs.ufs_inode;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/ufs/ufs/inode.h — UFS inode layout helpers.
export namespace pbsd::fs::ufs_inode {

inline constexpr unsigned kDirectBlocks = 12;
inline constexpr unsigned kNindir = 3;

struct Inode {
    unsigned short mode{};
    unsigned short nlink{};
    unsigned long long size{};
    unsigned direct[kDirectBlocks]{};
};

[[nodiscard]] inline Status validate_mode(unsigned short mode) noexcept {
    if ((mode & ~07777u) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_inode(const Inode& ino) noexcept {
    if (validate_mode(ino.mode) != Status::Ok) {
        return Status::Invalid;
    }
    if (ino.nlink == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_dir(unsigned short mode) noexcept {
    return (mode & 0170000u) == 0040000u;
}

} // namespace pbsd::fs::ufs_inode
""",
"fs/pbsd.fs.vfs_bio.cppm": """module;
#include <cstdint>

export module pbsd.fs.vfs_bio;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/vfs_bio.c — buffer cache states.
export namespace pbsd::fs::vfs_bio {

enum class State : unsigned char {
    Empty = 0,
    Busy  = 1,
    Done  = 2,
    Invalid = 3,
};

struct Buf {
    State state{State::Empty};
    unsigned bsize{};
    unsigned bcount{};
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::Empty:
    case State::Busy:
    case State::Done:
    case State::Invalid:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status start_io(Buf& b, unsigned count) noexcept {
    if (b.state != State::Empty || count == 0) {
        return Status::Invalid;
    }
    b.state = State::Busy;
    b.bcount = count;
    return Status::Ok;
}

[[nodiscard]] inline Status complete_io(Buf& b) noexcept {
    if (b.state != State::Busy) {
        return Status::Invalid;
    }
    b.state = State::Done;
    return Status::Ok;
}

} // namespace pbsd::fs::vfs_bio
""",
"fs/pbsd.fs.vfs_mount.cppm": """module;
#include <cstdint>

export module pbsd.fs.vfs_mount;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/vfs_mount.c — mount reference counting.
export namespace pbsd::fs::vfs_mount {

enum class Flag : unsigned {
    ReadOnly = 0x0001,
    Sync     = 0x0002,
    NoExec   = 0x0004,
};

struct Mount {
    unsigned refs{1};
    unsigned flags{};
    bool mounted{false};
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & ~(static_cast<unsigned>(Flag::ReadOnly) | static_cast<unsigned>(Flag::Sync)
                   | static_cast<unsigned>(Flag::NoExec))) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status acquire(Mount& m) noexcept {
    if (!m.mounted) {
        return Status::Invalid;
    }
    ++m.refs;
    return Status::Ok;
}

[[nodiscard]] inline Status release(Mount& m) noexcept {
    if (m.refs == 0) {
        return Status::Invalid;
    }
    --m.refs;
    return Status::Ok;
}

} // namespace pbsd::fs::vfs_mount
""",
"geom/pbsd.geom.bsd.cppm": """module;
#include <cstdint>

export module pbsd.geom.bsd;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/bsd/g_bsd.h — BSD disklabel slice constants.
export namespace pbsd::geom::bsd {

inline constexpr unsigned kMaxPartitions = 16;
inline constexpr unsigned kMagic = 0x82564557u;

struct Label {
    std::uint32_t magic{kMagic};
    unsigned short nparts{};
    unsigned long long disk_size{};
};

[[nodiscard]] inline Status validate_magic(std::uint32_t magic) noexcept {
    return magic == kMagic ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_label(const Label& lbl) noexcept {
    if (validate_magic(lbl.magic) != Status::Ok) {
        return Status::Invalid;
    }
    if (lbl.nparts == 0 || lbl.nparts > kMaxPartitions) {
        return Status::Invalid;
    }
    if (lbl.disk_size == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::bsd
""",
"geom/pbsd.geom.core.cppm": """module;
#include <cstdint>

export module pbsd.geom.core;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom.h — GEOM topology class flags.
export namespace pbsd::geom::core {

enum class ClassFlag : unsigned {
    Rank0 = 0x0001,
    Rank1 = 0x0002,
    Rank2 = 0x0004,
    Rank3 = 0x0008,
};

struct Class {
    const char* name{};
    unsigned rank{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_class(const Class& c) noexcept {
    if (c.name == nullptr || c.name[0] == '\\0') {
        return Status::Invalid;
    }
    if (c.rank > 3) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned rank_flag(unsigned rank) noexcept {
    return 1u << rank;
}

} // namespace pbsd::geom::core
""",
"geom/pbsd.geom.bsdlabel.cppm": """module;
#include <cstdint>

export module pbsd.geom.bsdlabel;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/bsd/g_bsd.c — BSD label provider geometry.
export namespace pbsd::geom::bsdlabel {

inline constexpr unsigned kSectorSize = 512;

struct Slice {
    unsigned long long offset{};
    unsigned long long length{};
    unsigned index{};
};

[[nodiscard]] inline Status validate_slice(const Slice& s) noexcept {
    if (s.length == 0) {
        return Status::Invalid;
    }
    if (s.offset % kSectorSize != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned long long end_offset(const Slice& s) noexcept {
    return s.offset + s.length;
}

} // namespace pbsd::geom::bsdlabel
""",
"zfs/pbsd.zfs.dsl_dir.cppm": """module;
#include <cstdint>

export module pbsd.zfs.dsl_dir;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dsl_dir.h — dataset directory.
export namespace pbsd::zfs::dsl_dir {

inline constexpr unsigned kNameMax = 256;

struct Phys {
    std::uint64_t used_bytes{};
    std::uint64_t compressed_bytes{};
    std::uint64_t quota{};
    std::uint64_t reservation{};
};

[[nodiscard]] inline Status validate_phys(const Phys& p) noexcept {
    if (p.quota != 0 && p.used_bytes > p.quota) {
        return Status::Invalid;
    }
    if (p.reservation != 0 && p.used_bytes < p.reservation) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t logical_used(const Phys& p) noexcept {
    return p.used_bytes;
}

} // namespace pbsd::zfs::dsl_dir
""",
"zfs/pbsd.zfs.dsl_dataset.cppm": """module;
#include <cstdint>

export module pbsd.zfs.dsl_dataset;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dsl_dataset.h — dataset head.
export namespace pbsd::zfs::dsl_dataset {

enum class Type : unsigned char {
    Filesystem = 1,
    Volume = 2,
    Snapshot = 3,
};

struct Head {
    Type type{Type::Filesystem};
    std::uint64_t objset{};
    std::uint64_t deadlist_obj{};
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    switch (t) {
    case Type::Filesystem:
    case Type::Volume:
    case Type::Snapshot:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_head(const Head& h) noexcept {
    if (validate_type(h.type) != Status::Ok) {
        return Status::Invalid;
    }
    if (h.objset == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::dsl_dataset
""",
"zfs/pbsd.zfs.vdev_label.cppm": """module;
#include <cstdint>

export module pbsd.zfs.vdev_label;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/vdev_label.h — vdev label count.
export namespace pbsd::zfs::vdev_label {

inline constexpr unsigned kLabels = 4;
inline constexpr unsigned kPadSize = 8192;

struct Config {
    std::uint64_t guid{};
    unsigned children{};
    unsigned ashift{9};
};

[[nodiscard]] inline Status validate_ashift(unsigned ashift) noexcept {
    if (ashift < 9 || ashift > 16) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_config(const Config& c) noexcept {
    if (c.guid == 0) {
        return Status::Invalid;
    }
    return validate_ashift(c.ashift);
}

[[nodiscard]] inline std::uint64_t label_offset(unsigned label_index,
                                                  std::uint64_t psize) noexcept {
    if (label_index >= kLabels) {
        return 0;
    }
    return label_index * (psize / kLabels);
}

} // namespace pbsd::zfs::vdev_label
""",
"zfs/pbsd.zfs.zil_header.cppm": """module;
#include <cstdint>

export module pbsd.zfs.zil_header;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zil_impl.h — ZIL block header.
export namespace pbsd::zfs::zil_header {

inline constexpr std::uint64_t kMagic = 0x2badbc2bULL;

struct Block {
    std::uint64_t magic{kMagic};
    std::uint64_t seq{};
    std::uint64_t birth{};
};

[[nodiscard]] inline Status validate_magic(std::uint64_t magic) noexcept {
    return magic == kMagic ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_block(const Block& b) noexcept {
    if (validate_magic(b.magic) != Status::Ok) {
        return Status::Invalid;
    }
    if (b.seq == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::zil_header
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
