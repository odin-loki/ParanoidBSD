module;

export module pbsd.fs.pseudofs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/pseudofs — synthetic vnode tree scaffold.
export namespace pbsd::fs::pseudofs {

enum class NodeKind : unsigned char {
    Dir = 0,
    File = 1,
    Link = 2,
};

struct Node {
    NodeKind kind{NodeKind::Dir};
    unsigned id{};
    bool readable{true};
};

[[nodiscard]] inline Status validate_node(const Node& n) noexcept {
    if (n.id == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status open_node(const Node& n) noexcept {
    if (validate_node(n) != Status::Ok || !n.readable) {
        return Status::Denied;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::pseudofs
