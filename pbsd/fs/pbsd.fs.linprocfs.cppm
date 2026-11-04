module;
#include <cstdint>

export module pbsd.fs.linprocfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/compat/linprocfs/linprocfs.c — Linux /proc compatibility nodes.
export namespace pbsd::fs::linprocfs {

inline constexpr unsigned kNameLen = 128;
inline constexpr unsigned kPidMax  = 99999;

enum class Node : unsigned char {
    Root = 0,
    Self = 1,
    Cpuinfo = 2,
    Meminfo = 3,
    Mounts = 4,
    Stat = 5,
    Uptime = 6,
    Loadavg = 7,
    Version = 8,
};

[[nodiscard]] inline Status validate_node(Node n) noexcept {
    switch (n) {
    case Node::Root:
    case Node::Self:
    case Node::Cpuinfo:
    case Node::Meminfo:
    case Node::Mounts:
    case Node::Stat:
    case Node::Uptime:
    case Node::Loadavg:
    case Node::Version:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_pid(unsigned pid) noexcept {
    if (pid == 0 || pid > kPidMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline char const* node_path(Node n) noexcept {
    switch (n) {
    case Node::Self:
        return "self";
    case Node::Cpuinfo:
        return "cpuinfo";
    case Node::Meminfo:
        return "meminfo";
    case Node::Mounts:
        return "mounts";
    case Node::Stat:
        return "stat";
    case Node::Uptime:
        return "uptime";
    case Node::Loadavg:
        return "loadavg";
    case Node::Version:
        return "version";
    default:
        return "";
    }
}

struct NodeLookup {
    Node   node{Node::Root};
    unsigned pid{};
};

[[nodiscard]] inline Status lookup(char const* name, NodeLookup& out) noexcept {
    if (name == nullptr) {
        return Status::Invalid;
    }
    if (name[0] == '\0') {
        out.node = Node::Root;
        out.pid = 0;
        return Status::Ok;
    }
    struct NamedNode {
        Node node;
        char const* path;
    };
    static constexpr NamedNode kNodes[] = {
        {Node::Self, "self"},
        {Node::Cpuinfo, "cpuinfo"},
        {Node::Meminfo, "meminfo"},
        {Node::Mounts, "mounts"},
        {Node::Stat, "stat"},
        {Node::Uptime, "uptime"},
        {Node::Loadavg, "loadavg"},
        {Node::Version, "version"},
    };
    for (NamedNode const& entry : kNodes) {
        char const* path = entry.path;
        unsigned i = 0;
        while (path[i] != '\0' && name[i] == path[i]) {
            ++i;
        }
        if (path[i] == '\0' && name[i] == '\0') {
            out.node = entry.node;
            out.pid = 0;
            return Status::Ok;
        }
    }
    unsigned pid = 0;
    unsigned i = 0;
    while (name[i] >= '0' && name[i] <= '9') {
        pid = pid * 10 + static_cast<unsigned>(name[i] - '0');
        ++i;
    }
    if (i == 0 || name[i] != '\0') {
        return Status::NotFound;
    }
    if (validate_pid(pid) != Status::Ok) {
        return Status::Invalid;
    }
    out.node = Node::Root;
    out.pid = pid;
    return Status::Ok;
}

} // namespace pbsd::fs::linprocfs
