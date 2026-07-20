module;
#include <cstdint>

export module pbsd.bifrost.hypercall;

import pbsd.core;

/// PROVENANCE: BIFROST HV — guest hypercall dispatch table scaffold.
export namespace pbsd::bifrost::hypercall {

enum class Call : unsigned char {
    Nop = 0,
    Yield = 1,
    Wallclock = 2,
    Shutdown = 3,
};

struct Request {
    Call call{Call::Nop};
    std::uint64_t arg0{};
    std::uint64_t arg1{};
};

struct Result {
    Status status{Status::Ok};
    std::uint64_t value{};
};

[[nodiscard]] inline Result dispatch(const Request& req) noexcept {
    Result r{};
    switch (req.call) {
    case Call::Nop:
        r.value = 0;
        return r;
    case Call::Yield:
        r.value = 1;
        return r;
    case Call::Wallclock:
        r.value = req.arg0;
        return r;
    case Call::Shutdown:
        r.status = Status::Ok;
        r.value = req.arg0;
        return r;
    }
    r.status = Status::Invalid;
    return r;
}

} // namespace pbsd::bifrost::hypercall
