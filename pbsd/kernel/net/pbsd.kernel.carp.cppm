export module pbsd.kernel.carp;

export import pbsd.core;

/// Wave 4 — CARP/VRRP kernel constants (netinet/ip_carp.h).
export namespace pbsd::kernel::carp {

inline constexpr unsigned kDfltTtl = 255;
inline constexpr unsigned kKeyLen = 20;
inline constexpr unsigned kDfltIntv = 1;
inline constexpr unsigned kMaxVhid = 255;
inline constexpr unsigned kMaxState = 2;
inline constexpr unsigned kMaxSkew = 240;

enum class Version : unsigned {
    Carp = 2,
    VrrpV3 = 3,
};

enum class State : int {
    Init = 0,
    Backup = 1,
    Master = 2,
};

enum class AdvType : unsigned char {
    Advertisement = 0x01,
};

struct Carpreq {
    int count{};
    int vhid{};
    State state{State::Init};
    int advskew{};
    int advbase{kDfltIntv};
    unsigned char key[kKeyLen]{};
};

[[nodiscard]] constexpr Status validate_vhid(int vhid) noexcept {
    if (vhid < 1 || vhid > static_cast<int>(kMaxVhid)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_advskew(int skew) noexcept {
    if (skew < 0 || skew > static_cast<int>(kMaxSkew)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_carpreq(const Carpreq& req) noexcept {
    if (validate_vhid(req.vhid) != Status::Ok) {
        return Status::Invalid;
    }
    if (validate_advskew(req.advskew) != Status::Ok) {
        return Status::Invalid;
    }
    if (static_cast<unsigned>(req.state) > kMaxState) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned state_table_size() noexcept {
    return 3;
}

} // namespace pbsd::kernel::carp
