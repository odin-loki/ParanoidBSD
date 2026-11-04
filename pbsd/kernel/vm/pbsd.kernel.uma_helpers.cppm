module;
#include <cstdint>

export module pbsd.kernel.uma_helpers;

import pbsd.core;
import pbsd.kernel.uma;

/// Freestanding uma(9) bucket/trim helpers from vm/uma_int.h policy surface.
export namespace pbsd::kernel::uma_helpers {

inline constexpr unsigned kUmaMaxWaste = 10u;
inline constexpr unsigned kDefaultBucketItems = 64u;
inline constexpr unsigned kMaxBucketItems     = 256u;

struct BucketParams {
    unsigned item_size{};
    unsigned zone_flags{};
    unsigned bucket_items{kDefaultBucketItems};
};

[[nodiscard]] constexpr unsigned waste_percent(unsigned item_size,
                                               unsigned slab_size) noexcept {
    if (slab_size == 0) {
        return 100u;
    }
    const unsigned aligned = uma::align_item_size(item_size);
    const unsigned per_slab = slab_size / aligned;
    if (per_slab == 0) {
        return 100u;
    }
    const unsigned used = per_slab * aligned;
    return ((slab_size - used) * 100u) / slab_size;
}

[[nodiscard]] constexpr bool use_buckets(const BucketParams& p) noexcept {
    if ((p.zone_flags & uma::kZoneNobucket) != 0) {
        return false;
    }
    if ((p.zone_flags & uma::kZoneMaxbucket) != 0) {
        return true;
    }
    return p.item_size <= 4096u;
}

[[nodiscard]] constexpr unsigned bucket_capacity(const BucketParams& p) noexcept {
    if ((p.zone_flags & uma::kZoneMaxbucket) != 0) {
        return kMaxBucketItems;
    }
    if (p.bucket_items == 0 || p.bucket_items > kMaxBucketItems) {
        return kDefaultBucketItems;
    }
    return p.bucket_items;
}

[[nodiscard]] inline Status trim_allowed(unsigned flags, unsigned free_slabs,
                                       unsigned min_slabs) noexcept {
    if ((flags & uma::kZoneNotrim) != 0) {
        return Status::Denied;
    }
    if (free_slabs <= min_slabs) {
        return Status::Denied;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool offpage_slab(unsigned item_size,
                                        unsigned slab_size) noexcept {
    return waste_percent(item_size, slab_size) > kUmaMaxWaste;
}

[[nodiscard]] inline Status validate_bucket_params(const BucketParams& p) noexcept {
    if (p.item_size == 0) {
        return Status::Invalid;
    }
    if (uma::validate_zone_flags(p.zone_flags) != Status::Ok) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::uma_helpers
