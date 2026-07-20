module;
#include <cstdint>

export module pbsd.stand.iscsi;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/iscsi/iscsi_proto.h — boot-time iSCSI PDU layout.
export namespace pbsd::stand::iscsi {

inline constexpr unsigned kBhsSize = 48;
inline constexpr unsigned kHeaderDigestSize = 4;
inline constexpr unsigned kDataDigestSize = 4;
inline constexpr unsigned kDefaultPort = 3260;
inline constexpr unsigned char kOpcodeImmediate = 0x40;

enum class Opcode : unsigned char {
    LoginRequest = 0x03,
    LoginResponse = 0x23,
    TextRequest = 0x04,
    TextResponse = 0x24,
    ScsiCommand = 0x01,
    ScsiResponse = 0x21,
    LogoutRequest = 0x06,
    LogoutResponse = 0x26,
};

struct BhsHint {
    Opcode opcode{Opcode::LoginRequest};
    bool immediate{false};
    unsigned ahs_len{0};
    unsigned data_len{0};
};

[[nodiscard]] inline unsigned char encode_opcode(Opcode op, bool immediate) noexcept {
    auto raw = static_cast<unsigned char>(op);
    if (immediate) {
        raw |= kOpcodeImmediate;
    }
    return raw;
}

[[nodiscard]] inline Status validate_bhs(const BhsHint& bhs) noexcept {
    if (bhs.ahs_len > kBhsSize) {
        return Status::Invalid;
    }
    if (bhs.data_len > 0xFFFFFFu) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    if (port == 0 || port > 65535u) {
        return Status::Invalid;
    }
    return Status::Ok;
}

struct BootTarget {
    const char* portal{nullptr};
    const char* target_name{nullptr};
    const char* lun{nullptr};
    unsigned port{kDefaultPort};
};

[[nodiscard]] inline Status validate_boot_target(const BootTarget& t) noexcept {
    if (t.portal == nullptr || t.target_name == nullptr) {
        return Status::Invalid;
    }
    return validate_port(t.port);
}

} // namespace pbsd::stand::iscsi
