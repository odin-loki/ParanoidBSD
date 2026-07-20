module;

export module pbsd.userland.libthr.stack;

import pbsd.core;

/// stack attributes from hbsd/src/lib/libthr/thread/thr_create.c
export namespace pbsd::userland::libthr {

inline constexpr unsigned kDefaultStackSize = 2u * 1024u * 1024u;
inline constexpr unsigned kMinStackSize = 64u * 1024u;

struct StackAttr {
    unsigned size{kDefaultStackSize};
    void* base{nullptr};
};

[[nodiscard]] inline Status stack_attr_set_size(StackAttr& attr, unsigned size) noexcept {
    if (size < kMinStackSize) {
        return Status::Invalid;
    }
    attr.size = size;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
