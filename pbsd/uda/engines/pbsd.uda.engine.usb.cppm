module;
#include <concepts>
#include <cstdint>

export module pbsd.uda.engine.usb;

export import pbsd.uda.interp;
import pbsd.core;

export namespace pbsd::uda {

enum class UsbTransferType : std::uint8_t {
    Control   = 0,
    Bulk      = 1,
    Interrupt = 2,
    Isochronous = 3,
};

struct UsbRequest {
    UsbTransferType type{};
    std::uint8_t    endpoint{};
    std::uint16_t   length{};
    std::uint8_t    address{};
};

class UsbEngine {
public:
    static constexpr std::size_t kMaxPending = 32;

    [[nodiscard]] static constexpr DeviceClass device_class() noexcept {
        return DeviceClass::Input;
    }

    explicit UsbEngine(SoftMmio& mem, DeviceGrant grant = {}) noexcept
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

    [[nodiscard]] Status submit(UsbRequest req) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (req.length == 0 || req.endpoint > 15) {
            return Status::Invalid;
        }
        if (pending_ >= kMaxPending) {
            return Status::Busy;
        }
        queue_[pending_++] = req;
        ++xfer_count_;
        return Status::Ok;
    }

    [[nodiscard]] Status complete(UsbRequest* out) noexcept {
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
        return Status::Ok;
    }

    [[nodiscard]] Status reset_port(std::uint8_t port) noexcept {
        if (!running_ || port == 0 || port > 127) {
            return Status::Invalid;
        }
        ++port_resets_;
        return Status::Ok;
    }

    [[nodiscard]] std::uint64_t xfer_count() const noexcept { return xfer_count_; }
    [[nodiscard]] std::uint64_t port_resets() const noexcept { return port_resets_; }

private:
    SoftMmio&         mem_;
    DeviceGrant       grant_;
    const Descriptor* desc_{};
    UsbRequest        queue_[kMaxPending]{};
    std::size_t       pending_{0};
    std::uint64_t     xfer_count_{0};
    std::uint64_t     port_resets_{0};
    bool              running_{false};
};

} // namespace pbsd::uda
