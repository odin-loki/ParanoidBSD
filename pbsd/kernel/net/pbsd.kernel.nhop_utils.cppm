module;
#include <cstdint>

export module pbsd.kernel.nhop_utils;

import pbsd.core;

/// Freestanding port of `net/route/nhop_utils.h` bitmask + chained-hash helpers.
export namespace pbsd::kernel::nhop_utils {

inline constexpr unsigned kBlockItems = 8 * sizeof(unsigned long);
inline constexpr unsigned kMaxItems   = 65536;
inline constexpr unsigned kMinBuckets = 16;

struct ChtHead {
    unsigned hash_size{};
    unsigned items_count{};
    void**   ptr{nullptr};
};

[[nodiscard]] inline unsigned cht_resize_buckets(const ChtHead& head) noexcept {
    if (head.items_count * 2 > head.hash_size && head.hash_size < kMaxItems) {
        return head.hash_size * 2;
    }
    if (head.items_count * 4 < head.hash_size && head.hash_size > kMinBuckets) {
        return head.hash_size / 2;
    }
    return 0;
}

[[nodiscard]] inline bool cht_need_resize(const ChtHead& head) noexcept {
    return cht_resize_buckets(head) > 0;
}

struct BitmaskHead {
    unsigned short free_off{};
    unsigned short blocks{};
    unsigned       items_count{};
    unsigned long* idx{nullptr};
};

[[nodiscard]] inline unsigned blocks_to_items(unsigned blocks) noexcept {
    return blocks * kBlockItems;
}

[[nodiscard]] inline unsigned items_to_blocks(unsigned items) noexcept {
    return items / kBlockItems;
}

[[nodiscard]] inline unsigned bitmask_resize_items(const BitmaskHead& bh) noexcept {
    if (bh.items_count * 2 > blocks_to_items(bh.blocks) &&
        bh.items_count < kMaxItems) {
        return blocks_to_items(bh.blocks) * 2;
    }
    return 0;
}

[[nodiscard]] inline bool bitmask_should_resize(const BitmaskHead& bh) noexcept {
    return bitmask_resize_items(bh) != 0;
}

inline void bitmask_init(BitmaskHead& bh, unsigned long* idx,
                         unsigned num_items) noexcept {
    bh.blocks = static_cast<unsigned short>(items_to_blocks(num_items));
    bh.items_count = num_items;
    bh.idx = idx;
    bh.free_off = 0;
}

[[nodiscard]] inline Status validate_items(unsigned num_items) noexcept {
    if (num_items == 0 || num_items > kMaxItems) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::nhop_utils
