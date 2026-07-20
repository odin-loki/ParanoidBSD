module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.sum;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cksum/sum1.c — 16-bit rotating checksum (logic-only).
export namespace pbsd::userland::usr_bin::sum {

struct SumResult {
    std::uint32_t checksum{0};
    std::uint64_t length{0};
};

[[nodiscard]] inline SumResult csum1_block(SumResult state, const unsigned char* buf,
                                           std::size_t len) noexcept {
    if (buf == nullptr) {
        return state;
    }
    std::uint32_t lcrc = state.checksum;
    for (std::size_t i = 0; i < len; ++i) {
        if ((lcrc & 1U) != 0U) {
            lcrc |= 0x10000U;
        }
        lcrc = ((lcrc >> 1) + buf[i]) & 0xffffU;
    }
    state.checksum = lcrc;
    state.length += len;
    return state;
}

[[nodiscard]] inline SumResult csum1_finish(SumResult state) noexcept {
    return state;
}

[[nodiscard]] inline bool format_bsd_sum(const SumResult& r, char* out, std::size_t outlen) noexcept {
    if (out == nullptr || outlen < 16) {
        return false;
    }
    const std::uint32_t c = r.checksum;
    const std::uint64_t blocks = (r.length + 1023ULL) / 1024ULL;
    int pos = 0;
    auto emit = [&](char ch) {
        if (pos + 1 >= static_cast<int>(outlen)) {
            return false;
        }
        out[pos++] = ch;
        return true;
    };
    auto emit_u = [&](std::uint64_t v) {
        char tmp[24];
        int n = 0;
        if (v == 0) {
            return emit('0');
        }
        while (v > 0) {
            tmp[n++] = static_cast<char>('0' + (v % 10));
            v /= 10;
        }
        while (n > 0) {
            if (!emit(tmp[--n])) {
                return false;
            }
        }
        return true;
    };
    if (!emit_u(c)) {
        return false;
    }
    if (!emit(' ')) {
        return false;
    }
    if (!emit_u(blocks)) {
        return false;
    }
    out[pos] = '\0';
    return true;
}

} // namespace pbsd::userland::usr_bin::sum
