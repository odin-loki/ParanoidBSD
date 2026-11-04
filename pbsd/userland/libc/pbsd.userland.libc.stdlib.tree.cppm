module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.tree;

/// tfind/tsearch/twalk/tdelete concepts from hbsd/src/lib/libc/stdlib/tree.c
export namespace pbsd::userland::libc {

using TreeCompare = int (*)(const void*, const void*) noexcept;
using TreeAction = void (*)(const void*, void*) noexcept;

struct TreeNode {
    const void* key{nullptr};
    TreeNode* left{nullptr};
    TreeNode* right{nullptr};
};

[[nodiscard]] inline TreeNode* tfind(const void* key, TreeNode* const* rootp,
                                     TreeCompare compar) noexcept {
    TreeNode* node = *rootp;
    while (node != nullptr) {
        const int cmp = compar(key, node->key);
        if (cmp == 0) {
            return node;
        }
        node = cmp < 0 ? node->left : node->right;
    }
    return nullptr;
}

[[nodiscard]] inline TreeNode* tsearch(const void* key, TreeNode** rootp,
                                       TreeCompare compar) noexcept {
    if (*rootp == nullptr) {
        *rootp = new TreeNode{key, nullptr, nullptr};
        return *rootp;
    }
    TreeNode* node = *rootp;
    for (;;) {
        const int cmp = compar(key, node->key);
        if (cmp == 0) {
            return node;
        }
        TreeNode** slot = cmp < 0 ? &node->left : &node->right;
        if (*slot == nullptr) {
            *slot = new TreeNode{key, nullptr, nullptr};
            return *slot;
        }
        node = *slot;
    }
}

inline void twalk(const TreeNode* root, TreeAction action, void* cookie) noexcept {
    if (root == nullptr) {
        return;
    }
    twalk(root->left, action, cookie);
    action(root->key, cookie);
    twalk(root->right, action, cookie);
}

[[nodiscard]] inline TreeNode* tdelete(const void* key, TreeNode** rootp,
                                       TreeCompare compar) noexcept {
    TreeNode** parent_link = rootp;
    TreeNode* node = *rootp;
    while (node != nullptr) {
        const int cmp = compar(key, node->key);
        if (cmp == 0) {
            if (node->left == nullptr) {
                TreeNode* victim = node;
                *parent_link = node->right;
                delete victim;
                return *parent_link;
            }
            if (node->right == nullptr) {
                TreeNode* victim = node;
                *parent_link = node->left;
                delete victim;
                return *parent_link;
            }
            TreeNode* succ = node->right;
            while (succ->left != nullptr) {
                succ = succ->left;
            }
            const void* tmp = node->key;
            node->key = succ->key;
            return tdelete(tmp, &node->right, compar);
        }
        parent_link = cmp < 0 ? &node->left : &node->right;
        node = *parent_link;
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
