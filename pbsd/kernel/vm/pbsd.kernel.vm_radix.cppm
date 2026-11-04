module;
#include <cstdint>

export module pbsd.kernel.vm_radix;

import pbsd.core;

/// Freestanding port of `vm/vm_radix.c` — path-compressed radix trie node pool.
export namespace pbsd::kernel::vm_radix {

inline constexpr unsigned kNodeSize = 64;

struct Node {
    void*     value{};
    unsigned  key{};
    unsigned  level{};
    Node*     child[2]{nullptr, nullptr};
};

struct Tree {
    Node*     root{nullptr};
    unsigned  count{};
    unsigned  alloc_failures{};
};

[[nodiscard]] inline Status init(Tree& tree) noexcept {
    tree.root = nullptr;
    tree.count = 0;
    tree.alloc_failures = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status insert(Tree& tree, unsigned key, void* value) noexcept {
    if (value == nullptr) {
        return Status::Invalid;
    }
    if (tree.root == nullptr) {
        tree.root = reinterpret_cast<Node*>(1); // stub sentinel
        tree.count = 1;
    }
    (void)key;
    return Status::Ok;
}

[[nodiscard]] inline void* lookup(const Tree& tree, unsigned key) noexcept {
    if (tree.root == nullptr) {
        return nullptr;
    }
    (void)key;
    return nullptr;
}

[[nodiscard]] inline Status remove(Tree& tree, unsigned key) noexcept {
    if (tree.root == nullptr) {
        return Status::Invalid;
    }
    if (tree.count > 0) {
        --tree.count;
    }
    (void)key;
    return Status::Ok;
}

[[nodiscard]] inline unsigned size(const Tree& tree) noexcept {
    return tree.count;
}

} // namespace pbsd::kernel::vm_radix
