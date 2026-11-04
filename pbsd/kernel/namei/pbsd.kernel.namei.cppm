export module pbsd.kernel.namei;

export import pbsd.core;

/// Wave 4 — namei lookup flags from sys/namei.h.
export namespace pbsd::kernel::namei {

enum class Op : unsigned char {
    Lookup = 0,
    Create = 1,
    Delete = 2,
    Rename = 3,
};

enum class Flags : unsigned {
    None = 0,
    LockLeaf = 0x0001,   // LOCKLEAF
    LockParent = 0x0002, // LOCKPARENT
    WantParent = 0x0010, // WANTPARENT
    NoFollow = 0x0008,   // NOFOLLOW
    Follow = 0x0004,     // FOLLOW
    NoTraverse = 0x0020, // NOTRIGGER / scaffold
    IsLastCn = 0x0040,   // ISSYMLINK handled elsewhere
    SaveStart = 0x0100,
    RdOnly = 0x0200,
};

[[nodiscard]] constexpr Flags operator|(Flags a, Flags b) noexcept {
    return static_cast<Flags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool has(Flags set, Flags bit) noexcept {
    return (static_cast<unsigned>(set) & static_cast<unsigned>(bit)) != 0;
}

struct ComponentName {
    Op op{Op::Lookup};
    Flags flags{Flags::None};
    const char* nameptr{nullptr};
    unsigned namelen{};
};

[[nodiscard]] constexpr Status validate(const ComponentName& cn) noexcept {
    if (cn.nameptr == nullptr && cn.namelen != 0) {
        return Status::Invalid;
    }
    if (has(cn.flags, Flags::Follow) && has(cn.flags, Flags::NoFollow)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::namei
