export module pbsd.net.ipsec;

export import pbsd.core;

/// Wave 6 — IPsec SA/SP stubs from netipsec/ipsec.h.
export namespace pbsd::net::ipsec {

enum class Proto : unsigned char {
    Esp = 50,
    Ah  = 51,
    Ipcomp = 108,
};

enum class Mode : unsigned char {
    Transport = 0,
    Tunnel    = 1,
};

enum class Dir : unsigned char {
    In  = 0,
    Out = 1,
    Fwd = 2,
};

struct SaStub {
    Proto proto{Proto::Esp};
    Mode mode{Mode::Transport};
    unsigned spi{};
    unsigned reqid{};
};

[[nodiscard]] constexpr Status validate_sa(const SaStub& sa) noexcept {
    if (sa.spi == 0 || sa.spi == 0xffffffffu) {
        return Status::Invalid;
    }
    if (sa.proto != Proto::Esp && sa.proto != Proto::Ah && sa.proto != Proto::Ipcomp) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ipsec
