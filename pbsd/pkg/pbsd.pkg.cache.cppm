module;
#include <cstdint>

export module pbsd.pkg.cache;

import pbsd.core;

/// Burst 12 — local package cache slot metadata.
export namespace pbsd::pkg::cache {

inline constexpr unsigned kMaxSlots = 64;
inline constexpr unsigned kDigestLen = 65;

enum class SlotState : unsigned char {
    Empty   = 0,
    Staged  = 1,
    Ready   = 2,
    Evicted = 3,
};

struct CacheSlot {
    SlotState state{SlotState::Empty};
    char digest_hex[kDigestLen]{};
    unsigned long long size{};
    unsigned ref_count{};
};

struct CacheTable {
    unsigned slot_count{0};
    unsigned long long total_bytes{};
    CacheSlot slots[kMaxSlots]{};
};

[[nodiscard]] inline Status find_slot(CacheTable const& tbl, char const* digest_hex,
                                      unsigned& out_idx) noexcept {
    if (digest_hex == nullptr || digest_hex[0] == '\0') {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < tbl.slot_count; ++i) {
        if (tbl.slots[i].state == SlotState::Empty) {
            continue;
        }
        bool match = true;
        for (unsigned j = 0; digest_hex[j] != '\0' || tbl.slots[i].digest_hex[j] != '\0'; ++j) {
            if (digest_hex[j] != tbl.slots[i].digest_hex[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            out_idx = i;
            return Status::Ok;
        }
    }
    return Status::NotFound;
}

[[nodiscard]] inline Status stage(CacheTable& tbl, char const* digest_hex,
                                unsigned long long size, unsigned& out_idx) noexcept {
    if (tbl.slot_count >= kMaxSlots) {
        return Status::Busy;
    }
    out_idx = tbl.slot_count;
    auto& slot = tbl.slots[out_idx];
    slot.state = SlotState::Staged;
    for (unsigned i = 0; digest_hex[i] != '\0' && i < kDigestLen - 1; ++i) {
        slot.digest_hex[i] = digest_hex[i];
    }
    slot.size = size;
    slot.ref_count = 1;
    tbl.slot_count++;
    tbl.total_bytes += size;
    return Status::Ok;
}

} // namespace pbsd::pkg::cache
