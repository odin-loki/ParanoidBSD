module;
#include <cstdint>

export module pbsd.zfs.sa;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/sa.h — system attributes.
export namespace pbsd::zfs::sa {

inline constexpr unsigned kAttrMaxLen = 65535;

enum class BswapType : unsigned char {
    Uint64Array = 0,
    Uint32Array = 1,
    Uint16Array = 2,
    Uint8Array  = 3,
    Acl         = 4,
};

enum class HandleType : unsigned char {
    Shared  = 0,
    Private = 1,
};

struct AttrReg {
    const char* name{};
    unsigned short length{};
    BswapType byteswap{BswapType::Uint8Array};
    unsigned short attr{};
};

struct BulkAttr {
    void* data{};
    unsigned short length{};
    unsigned short attr{};
    unsigned short buftype{};
};

[[nodiscard]] inline Status validate_length(unsigned len) noexcept {
    if (len == 0 || len > kAttrMaxLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_attr_reg(AttrReg const& reg) noexcept {
    if (reg.name == nullptr) {
        return Status::Invalid;
    }
    return validate_length(reg.length);
}

[[nodiscard]] inline Status add_bulk_attr(BulkAttr& entry, unsigned short attr,
                                          void* data, unsigned short len) noexcept {
    if (validate_length(len) != Status::Ok) {
        return Status::Invalid;
    }
    entry.attr = attr;
    entry.data = data;
    entry.length = len;
    return Status::Ok;
}

[[nodiscard]] inline Status validate_handle_type(HandleType t) noexcept {
    if (t != HandleType::Shared && t != HandleType::Private) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::sa
