module;
#include <concepts>
#include <cstdint>

export module pbsd.uda.engine.crypto;

export import pbsd.uda.interp;
import pbsd.core;

export namespace pbsd::uda {

enum class CryptoOp : std::uint8_t {
    Encrypt = 0,
    Decrypt = 1,
    Hash    = 2,
    Hmac    = 3,
};

struct CryptoRequest {
    CryptoOp      op{};
    std::uint16_t key_slot{};
    std::uint32_t length{};
    std::uint32_t iv{};
};

class CryptoEngine {
public:
    static constexpr std::size_t kMaxPending = 16;

    [[nodiscard]] static constexpr DeviceClass device_class() noexcept {
        return DeviceClass::Block;
    }

    explicit CryptoEngine(SoftMmio& mem, DeviceGrant grant = {}) noexcept
        : mem_(mem), grant_(grant.mmio_valid() ? grant
                                               : DeviceGrant{{0, SoftMmio::kSize}, {}}) {}

    [[nodiscard]] Status probe(const Descriptor& d) noexcept {
        if (validate_descriptor(d) != Status::Ok) {
            return Status::Invalid;
        }
        desc_ = &d;
        return Status::Ok;
    }

    [[nodiscard]] Status start() noexcept {
        if (!desc_) {
            return Status::Invalid;
        }
        BytecodeInterpreter interp{grant_, mem_};
        const auto st = interp.run(desc_->init_sequence);
        if (st == Status::Ok) {
            running_ = true;
        }
        return st;
    }

    [[nodiscard]] Status stop() noexcept {
        if (!desc_) {
            return Status::Invalid;
        }
        running_ = false;
        pending_ = 0;
        BytecodeInterpreter interp{grant_, mem_};
        return interp.run(desc_->reset_sequence);
    }

    [[nodiscard]] DeviceGrant grant() const noexcept { return grant_; }

    [[nodiscard]] Status submit(CryptoRequest req) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (req.length == 0 || req.length > 65536) {
            return Status::Invalid;
        }
        if (pending_ >= kMaxPending) {
            return Status::Busy;
        }
        queue_[pending_++] = req;
        ++submit_count_;
        return Status::Ok;
    }

    [[nodiscard]] Status complete(CryptoRequest* out) noexcept {
        if (!running_ || pending_ == 0) {
            return Status::NotFound;
        }
        if (out) {
            *out = queue_[0];
        }
        for (std::size_t i = 1; i < pending_; ++i) {
            queue_[i - 1] = queue_[i];
        }
        --pending_;
        ++complete_count_;
        return Status::Ok;
    }

    [[nodiscard]] std::uint64_t submit_count() const noexcept { return submit_count_; }
    [[nodiscard]] std::uint64_t complete_count() const noexcept { return complete_count_; }

private:
    SoftMmio&         mem_;
    DeviceGrant       grant_;
    const Descriptor* desc_{};
    CryptoRequest     queue_[kMaxPending]{};
    std::size_t       pending_{0};
    std::uint64_t     submit_count_{0};
    std::uint64_t     complete_count_{0};
    bool              running_{false};
};

} // namespace pbsd::uda
