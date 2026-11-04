module;
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
