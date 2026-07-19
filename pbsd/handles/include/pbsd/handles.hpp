// PBSD — Kernel Handle & Capability Type Hierarchy
// Author: Odin Loch
// PROVENANCE: Original design; no third-party source derived.
// All ownership-sensitive kernel state MUST route through these types.
// The static analyser reasons about this closed type set only.

#pragma once
#include <cstdint>
#include <utility>

namespace pbsd {

// CapabilityRights — rights bitmask, only narrows on duplication/grant
enum class CapabilityRights : uint32_t {
    None      = 0,
    Read      = 1 << 0,
    Write     = 1 << 1,
    Execute   = 1 << 2,
    Grant     = 1 << 3,
    Duplicate = 1 << 4,
    Destroy   = 1 << 5,
    All       = 0x3F,
};

inline constexpr CapabilityRights operator&(CapabilityRights a, CapabilityRights b) {
    return static_cast<CapabilityRights>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline constexpr bool has_right(CapabilityRights set, CapabilityRights right) {
    return (set & right) != CapabilityRights::None;
}

// UniqueHandle<T> — move-only, single owner.
// [[kernel::owns]] annotation — static analyser tracks ownership state.
template<typename T>
class [[nodiscard]] UniqueHandle {
public:
    UniqueHandle() noexcept = default;

    explicit UniqueHandle(T* ptr, CapabilityRights rights) noexcept
        : ptr_(ptr), rights_(rights) {}

    // Move only
    UniqueHandle(UniqueHandle&& other) noexcept
        : ptr_(other.ptr_), rights_(other.rights_) {
        other.ptr_ = nullptr;
        other.rights_ = CapabilityRights::None;
    }

    UniqueHandle& operator=(UniqueHandle&& other) noexcept {
        if (this != &other) {
            reset();
            ptr_ = other.ptr_;
            rights_ = other.rights_;
            other.ptr_ = nullptr;
            other.rights_ = CapabilityRights::None;
        }
        return *this;
    }

    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;

    ~UniqueHandle() noexcept { reset(); }

    [[nodiscard]] bool valid() const noexcept { return ptr_ != nullptr; }
    [[nodiscard]] bool has_right(CapabilityRights r) const noexcept {
        return pbsd::has_right(rights_, r);
    }

    // Narrow rights only — returns a new handle with reduced rights
    [[nodiscard]] UniqueHandle narrow(CapabilityRights new_rights) && noexcept {
        // Rights can only narrow, not widen
        CapabilityRights narrowed = rights_ & new_rights;
        UniqueHandle h(ptr_, narrowed);
        ptr_ = nullptr;
        rights_ = CapabilityRights::None;
        return h;
    }

    void reset() noexcept {
        if (ptr_) {
            // Kernel object release hook — T must provide static T::release()
            T::release(ptr_);
            ptr_ = nullptr;
            rights_ = CapabilityRights::None;
        }
    }

    T* get() const noexcept { return ptr_; }

private:
    T* ptr_ = nullptr;
    CapabilityRights rights_ = CapabilityRights::None;
};

// BorrowedHandle<T> — non-owning view, [[kernel::no_escape]].
// Must not outlive the UniqueHandle or SharedHandle that produced it.
template<typename T>
class BorrowedHandle {
public:
    BorrowedHandle() noexcept = default;

    explicit BorrowedHandle(T* ptr, CapabilityRights rights) noexcept
        : ptr_(ptr), rights_(rights) {}

    [[nodiscard]] bool valid() const noexcept { return ptr_ != nullptr; }
    [[nodiscard]] bool has_right(CapabilityRights r) const noexcept {
        return pbsd::has_right(rights_, r);
    }
    T* get() const noexcept { return ptr_; }

    // BorrowedHandle is copyable (multiple borrows are fine)
    BorrowedHandle(const BorrowedHandle&) noexcept = default;
    BorrowedHandle& operator=(const BorrowedHandle&) noexcept = default;

private:
    T* ptr_ = nullptr;
    CapabilityRights rights_ = CapabilityRights::None;
};

// Helper: produce a BorrowedHandle from a UniqueHandle
// The borrow MUST NOT outlive the UniqueHandle — enforced by static analyser
template<typename T>
[[nodiscard]] BorrowedHandle<T> borrow(const UniqueHandle<T>& h) noexcept {
    return BorrowedHandle<T>(h.get(), CapabilityRights::Read);
}

} // namespace pbsd
