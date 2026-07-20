module;
#include <cstdint>

export module pbsd.kernel.capsicum;

import pbsd.core;

/// Freestanding port of hbsd `subr_capability.c` + `sys_capability.c` rights algebra.
/// Constants and layout match `sys/capsicum.h` / `sys/caprights.h`.
export namespace pbsd::kernel::capsicum {

inline constexpr int kCapRightsVersion00 = 0;
inline constexpr int kCapRightsVersion   = kCapRightsVersion00;
inline constexpr int kCapArsizeMin       = kCapRightsVersion00 + 2;
inline constexpr int kCapArsizeMax       = kCapRightsVersion + 2;

// sys/capability.h — capmode / cap rights limit errno bridge targets
inline constexpr int kCapModeEnabled = 1;
inline constexpr int kCapModeDisabled = 0;

[[nodiscard]] constexpr std::uint64_t capright(int idx, std::uint64_t bit) noexcept {
    return (1ull << (57 + idx)) | bit;
}

// INDEX 0 — hbsd capsicum.h
inline constexpr std::uint64_t kCapRead  = capright(0, 0x0000000000000001ull);
inline constexpr std::uint64_t kCapWrite = capright(0, 0x0000000000000002ull);
inline constexpr std::uint64_t kCapSeekTell = capright(0, 0x0000000000000004ull);
inline constexpr std::uint64_t kCapSeek     = kCapSeekTell | 0x0000000000000008ull;
inline constexpr std::uint64_t kCapPread    = kCapSeek | kCapRead;
inline constexpr std::uint64_t kCapPwrite   = kCapSeek | kCapWrite;
inline constexpr std::uint64_t kCapMmap    = capright(0, 0x0000000000000010ull);
inline constexpr std::uint64_t kCapMmapR    = kCapMmap | kCapSeek | kCapRead;
inline constexpr std::uint64_t kCapMmapW    = kCapMmap | kCapSeek | kCapWrite;
inline constexpr std::uint64_t kCapMmapX    = kCapMmap | kCapSeek | 0x0000000000000020ull;
inline constexpr std::uint64_t kCapMmapRw   = kCapMmapR | kCapMmapW;
inline constexpr std::uint64_t kCapMmapRx   = kCapMmapR | kCapMmapX;
inline constexpr std::uint64_t kCapMmapWx   = kCapMmapW | kCapMmapX;
inline constexpr std::uint64_t kCapMmapRwx  = kCapMmapR | kCapMmapW | kCapMmapX;
inline constexpr std::uint64_t kCapCreate   = capright(0, 0x0000000000000040ull);
inline constexpr std::uint64_t kCapFexecve  = capright(0, 0x0000000000000080ull);
inline constexpr std::uint64_t kCapFsync    = capright(0, 0x0000000000000100ull);
inline constexpr std::uint64_t kCapFtruncate = capright(0, 0x0000000000000200ull);
inline constexpr std::uint64_t kCapLookup   = capright(0, 0x0000000000000400ull);
inline constexpr std::uint64_t kCapAccept   = capright(0, 0x0000000020000000ull);
inline constexpr std::uint64_t kCapBind     = capright(0, 0x0000000040000000ull);
inline constexpr std::uint64_t kCapConnect  = capright(0, 0x0000000080000000ull);
inline constexpr std::uint64_t kCapRecv     = kCapRead;
inline constexpr std::uint64_t kCapSend     = kCapWrite;
inline constexpr std::uint64_t kCapAll0     = capright(0, 0x00000FFFFFFFFFull);

// INDEX 1 — capsicum.h
inline constexpr std::uint64_t kCapMacGet   = capright(1, 0x0000000000000001ull);
inline constexpr std::uint64_t kCapMacSet   = capright(1, 0x0000000000000002ull);
inline constexpr std::uint64_t kCapEvent    = capright(1, 0x0000000000000020ull);
inline constexpr std::uint64_t kCapIoctl    = capright(1, 0x0000000000000080ull);
inline constexpr std::uint64_t kCapPdgetpid = capright(1, 0x0000000000000200ull);
inline constexpr std::uint64_t kCapAll1 = capright(1, 0x00000000007FFFFFull);

struct CapRights {
    std::uint64_t cr_rights[kCapArsizeMax]{};

    [[nodiscard]] constexpr int version() const noexcept {
        return static_cast<int>(cr_rights[0] >> 62);
    }

    [[nodiscard]] constexpr unsigned arsize() const noexcept {
        return static_cast<unsigned>(version() + 2);
    }
};

[[nodiscard]] constexpr int capidxbit(std::uint64_t right) noexcept {
    return static_cast<int>((right >> 57) & 0x1F);
}

[[nodiscard]] constexpr int caprver(std::uint64_t right) noexcept {
    return static_cast<int>(right >> 62);
}

[[nodiscard]] constexpr CapRights cap_none() noexcept {
    CapRights r{};
    r.cr_rights[0] = (static_cast<std::uint64_t>(kCapRightsVersion) << 62)
                   | capright(0, 0ull);
    r.cr_rights[1] = capright(1, 0ull);
    return r;
}

[[nodiscard]] inline CapRights cap_all() noexcept {
    CapRights r{};
    r.cr_rights[0] = (static_cast<std::uint64_t>(kCapRightsVersion) << 62) | kCapAll0;
    r.cr_rights[1] = kCapAll1;
    return r;
}

/// `subr_capability.c:right_to_index` bit2idx table.
[[nodiscard]] constexpr int right_to_index(std::uint64_t right) noexcept {
    static constexpr int bit2idx[] = {
        -1, 0, 1, -1, 2, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1,
        4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };
    const int idx = capidxbit(right);
    if (idx < 0 || idx >= static_cast<int>(sizeof(bit2idx) / sizeof(bit2idx[0]))) {
        return -1;
    }
    return bit2idx[idx];
}

[[nodiscard]] constexpr bool cap_rights_is_set(const CapRights& rights,
                                            std::uint64_t right) noexcept {
    if (rights.version() != kCapRightsVersion00) {
        return false;
    }
    const int i = right_to_index(right);
    if (i < 0 || static_cast<unsigned>(i) >= rights.arsize()) {
        return false;
    }
    return (rights.cr_rights[i] & right) == right;
}

[[nodiscard]] constexpr bool cap_rights_contains(const CapRights& big,
                                              const CapRights& little) noexcept {
    if (big.version() != kCapRightsVersion00 || little.version() != kCapRightsVersion00) {
        return false;
    }
    if (big.version() != little.version()) {
        return false;
    }
    const unsigned n = big.arsize();
    if (n < kCapArsizeMin || n > kCapArsizeMax) {
        return false;
    }
    for (unsigned i = 0; i < n; ++i) {
        if ((big.cr_rights[i] & little.cr_rights[i]) != little.cr_rights[i]) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool cap_rights_is_empty(const CapRights& rights) noexcept {
    const CapRights none = cap_none();
    const unsigned n = rights.arsize();
    for (unsigned i = 0; i < n; ++i) {
        if (rights.cr_rights[i] != none.cr_rights[i]) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool cap_rights_is_valid(const CapRights& rights) noexcept {
    if (rights.version() != kCapRightsVersion00) {
        return false;
    }
    const unsigned n = rights.arsize();
    if (n < kCapArsizeMin || n > kCapArsizeMax) {
        return false;
    }
    const CapRights all = cap_all();
    if (!cap_rights_contains(all, rights)) {
        return false;
    }
    for (unsigned i = 0; i < n; ++i) {
        const int j = right_to_index(rights.cr_rights[i]);
        if (static_cast<unsigned>(j) != i) {
            return false;
        }
        if (i > 0 && caprver(rights.cr_rights[i]) != 0) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline CapRights cap_rights_merge(CapRights dst, const CapRights& src) noexcept {
    const unsigned n = dst.arsize();
    for (unsigned i = 0; i < n; ++i) {
        dst.cr_rights[i] |= src.cr_rights[i];
    }
    return dst;
}

[[nodiscard]] inline CapRights cap_rights_remove(CapRights dst, const CapRights& src) noexcept {
    const unsigned n = dst.arsize();
    for (unsigned i = 0; i < n; ++i) {
        dst.cr_rights[i] &= ~(src.cr_rights[i] & 0x01FFFFFFFFFFFFFFull);
    }
    return dst;
}

[[nodiscard]] constexpr CapRights cap_rights_set_one(CapRights dst, std::uint64_t right) noexcept {
    const int i = right_to_index(right);
    if (i >= 0) {
        dst.cr_rights[i] |= right;
    }
    return dst;
}

/// `sys_capability.c:cap_check` — subset test (ENOTCAPABLE → Status::Denied).
[[nodiscard]] inline Status cap_check(const CapRights& have, const CapRights& need) noexcept {
    return cap_rights_contains(have, need) ? Status::Ok : Status::Denied;
}

/// `kern_cap_rights_limit` / `_cap_check(..., CAPFAIL_INCREASE)` — rights must narrow.
[[nodiscard]] inline Status cap_rights_limit(const CapRights& current,
                                             const CapRights& limit) noexcept {
    if (!cap_rights_is_valid(limit)) {
        return Status::Invalid;
    }
    return cap_check(current, limit);
}

/// `sys_capability.c:cap_rights_to_vmprot` — VM_PROT_* bit mapping.
[[nodiscard]] inline unsigned char cap_rights_to_vmprot(const CapRights& have) noexcept {
    unsigned char maxprot = 0;
    if (cap_rights_is_set(have, kCapMmapR)) {
        maxprot |= 0x01u; // VM_PROT_READ
    }
    if (cap_rights_is_set(have, kCapMmapW)) {
        maxprot |= 0x02u; // VM_PROT_WRITE
    }
    if (cap_rights_is_set(have, kCapMmapX)) {
        maxprot |= 0x04u; // VM_PROT_EXECUTE
    }
    return maxprot;
}

/// Predefined rights tables from `subr_capability.c` (kernel extern const).
struct CapRightsTable {
    CapRights read{};
    CapRights write{};
    CapRights mmap{};
    CapRights mmap_r{};
    CapRights mmap_w{};
    CapRights mmap_x{};
    CapRights mmap_rw{};
    CapRights mmap_rx{};
    CapRights mmap_wx{};
    CapRights mmap_rwx{};
    CapRights accept{};
    CapRights bind{};
    CapRights connect{};
    CapRights event{};
    CapRights ioctl{};
    CapRights pdgetpid{};
    CapRights send_connect{};
    CapRights none{};

    [[nodiscard]] static unsigned table_entry_count() noexcept {
        return 18u;
    }

    [[nodiscard]] static constexpr CapRightsTable make() noexcept {
        CapRightsTable t{};
        auto one = [](std::uint64_t r) {
            return cap_rights_set_one(cap_none(), r);
        };
        t.read          = one(kCapRead);
        t.write         = one(kCapWrite);
        t.mmap          = one(kCapMmap);
        t.mmap_r        = one(kCapMmapR);
        t.mmap_w        = one(kCapMmapW);
        t.mmap_x        = one(kCapMmapX);
        t.mmap_rw       = one(kCapMmapRw);
        t.mmap_rx       = one(kCapMmapRx);
        t.mmap_wx       = one(kCapMmapWx);
        t.mmap_rwx      = one(kCapMmapRwx);
        t.accept        = one(kCapAccept);
        t.bind          = one(kCapBind);
        t.connect       = one(kCapConnect);
        t.event         = one(kCapEvent);
        t.ioctl         = one(kCapIoctl);
        t.pdgetpid      = one(kCapPdgetpid);
        t.send_connect  = cap_rights_set_one(cap_rights_set_one(cap_none(), kCapSend), kCapConnect);
        t.none          = cap_none();
        return t;
    }
};

namespace proofs {
static_assert(kCapMmapRwx == (kCapMmapR | kCapMmapW | kCapMmapX));
static_assert(cap_rights_is_set(CapRightsTable::make().mmap_r, kCapMmapR));
static_assert(!cap_rights_contains(CapRightsTable::make().read, CapRightsTable::make().write));
} // namespace proofs

} // namespace pbsd::kernel::capsicum
