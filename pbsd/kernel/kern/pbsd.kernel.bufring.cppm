module;
#include <cstdint>

export module pbsd.kernel.bufring;

import pbsd.core;

/// Freestanding port of `sys/buf_ring.h` / `kern/subr_bufring.c`.
export namespace pbsd::kernel::bufring {

inline constexpr unsigned kMaxSlots = 4096;

struct Ring {
    int           prod_size{};
    int           cons_size{};
    unsigned      prod_mask{};
    unsigned      cons_mask{};
    unsigned      prod_head{};
    unsigned      cons_head{};
    unsigned      prod_tail{};
    unsigned      cons_tail{};
};

[[nodiscard]] inline bool is_power_of_two(int count) noexcept {
    return count > 0 && (count & (count - 1)) == 0;
}

[[nodiscard]] inline Status init(Ring& br, int count) noexcept {
    if (!is_power_of_two(count) || count > static_cast<int>(kMaxSlots)) {
        return Status::Invalid;
    }
    br.prod_size = count;
    br.cons_size = count;
    br.prod_mask = static_cast<unsigned>(count - 1);
    br.cons_mask = static_cast<unsigned>(count - 1);
    br.prod_head = 0;
    br.cons_head = 0;
    br.prod_tail = 0;
    br.cons_tail = 0;
    return Status::Ok;
}

[[nodiscard]] inline unsigned prod_capacity(const Ring& br) noexcept {
    return static_cast<unsigned>(br.prod_size);
}

[[nodiscard]] inline unsigned prod_occupancy(const Ring& br) noexcept {
    const unsigned cap = prod_capacity(br);
    if (cap == 0) {
        return 0;
    }
    return (br.prod_head + cap - br.cons_tail) & br.prod_mask;
}

[[nodiscard]] inline bool prod_full(const Ring& br) noexcept {
    return prod_occupancy(br) >= prod_capacity(br);
}

[[nodiscard]] inline bool prod_empty(const Ring& br) noexcept {
    return br.prod_head == br.cons_tail;
}

[[nodiscard]] inline Status enqueue(Ring& br) noexcept {
    if (prod_full(br)) {
        return Status::Denied;
    }
    br.prod_head = (br.prod_head + 1) & br.prod_mask;
    return Status::Ok;
}

[[nodiscard]] inline Status dequeue(Ring& br) noexcept {
    if (prod_empty(br)) {
        return Status::Invalid;
    }
    br.cons_tail = (br.cons_tail + 1) & br.cons_mask;
    return Status::Ok;
}

} // namespace pbsd::kernel::bufring
