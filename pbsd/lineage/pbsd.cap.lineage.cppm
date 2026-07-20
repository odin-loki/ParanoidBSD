export module pbsd.cap.lineage;

import pbsd.core;

export namespace pbsd {

using LineageId = unsigned long long;

inline constexpr LineageId kInvalidLineage = 0;

/// Hosted/freestanding-friendly revocation table (fixed capacity for nucleus).
class LineageTree {
public:
    static constexpr unsigned kMaxNodes = 4096;

    [[nodiscard]] LineageId create_root() noexcept {
        return alloc_node(kInvalidLineage);
    }

    [[nodiscard]] LineageId create_child(LineageId parent) noexcept {
        if (!valid_unrevoked(parent)) {
            return kInvalidLineage;
        }
        return alloc_node(parent);
    }

    [[nodiscard]] bool valid(LineageId id) const noexcept {
        return valid_unrevoked(id);
    }

    [[nodiscard]] bool is_revoked(LineageId id) const noexcept {
        if (id == kInvalidLineage || id > next_id_) {
            return true;
        }
        return revoked_[index_of(id)];
    }

    [[nodiscard]] LineageId parent(LineageId id) const noexcept {
        if (id == kInvalidLineage || id > next_id_) {
            return kInvalidLineage;
        }
        return parent_[index_of(id)];
    }

    [[nodiscard]] bool is_ancestor_of(LineageId ancestor, LineageId node) const noexcept {
        if (ancestor == kInvalidLineage || node == kInvalidLineage) {
            return false;
        }
        return is_descendant(node, ancestor);
    }

    /// SI-6: revoke node and all descendants atomically from holder perspective.
    void revoke_subtree(LineageId id) noexcept {
        if (id == kInvalidLineage || id > next_id_) {
            return;
        }
        for (LineageId i = 1; i <= next_id_; ++i) {
            if (i == id || is_descendant(i, id)) {
                revoked_[index_of(i)] = true;
            }
        }
    }

    void revoke(LineageId id) noexcept { revoke_subtree(id); }

private:
    LineageId next_id_{0};
    LineageId parent_[kMaxNodes]{};
    bool revoked_[kMaxNodes]{};

    [[nodiscard]] static constexpr unsigned index_of(LineageId id) noexcept {
        return static_cast<unsigned>(id - 1);
    }

    [[nodiscard]] bool valid_unrevoked(LineageId id) const noexcept {
        if (id == kInvalidLineage || id > next_id_) {
            return false;
        }
        return !revoked_[index_of(id)];
    }

    [[nodiscard]] LineageId alloc_node(LineageId parent) noexcept {
        if (next_id_ >= kMaxNodes) {
            return kInvalidLineage;
        }
        ++next_id_;
        parent_[index_of(next_id_)] = parent;
        revoked_[index_of(next_id_)] = false;
        return next_id_;
    }

    [[nodiscard]] bool is_descendant(LineageId node, LineageId ancestor) const noexcept {
        LineageId cur = node;
        while (cur != kInvalidLineage) {
            if (cur == ancestor) {
                return true;
            }
            cur = parent_[index_of(cur)];
        }
        return false;
    }
};

} // namespace pbsd
