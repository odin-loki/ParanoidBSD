module;
#include <concepts>
#include <cstdint>

export module pbsd.uda.engine.pci;

export import pbsd.uda.interp;
import pbsd.core;

export namespace pbsd::uda {

struct PciFunction {
    std::uint8_t  bus{};
    std::uint8_t  slot{};
    std::uint8_t  func{};
    std::uint16_t vendor_id{};
    std::uint16_t device_id{};
    std::uint8_t  class_code{};
};

class PciEngine {
public:
    static constexpr std::size_t kMaxFunctions = 64;

    [[nodiscard]] static constexpr DeviceClass device_class() noexcept {
        return DeviceClass::Block;
    }

    explicit PciEngine(SoftMmio& mem, DeviceGrant grant = {}) noexcept
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
        function_count_ = 0;
        BytecodeInterpreter interp{grant_, mem_};
        return interp.run(desc_->reset_sequence);
    }

    [[nodiscard]] DeviceGrant grant() const noexcept { return grant_; }

    [[nodiscard]] Status register_function(PciFunction fn) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (fn.vendor_id == 0xFFFF) {
            return Status::Invalid;
        }
        if (function_count_ >= kMaxFunctions) {
            return Status::Busy;
        }
        functions_[function_count_++] = fn;
        return Status::Ok;
    }

    [[nodiscard]] Status find_by_vendor(std::uint16_t vendor_id,
                                        PciFunction* out) const noexcept {
        if (!running_ || out == nullptr) {
            return Status::Invalid;
        }
        for (std::size_t i = 0; i < function_count_; ++i) {
            if (functions_[i].vendor_id == vendor_id) {
                *out = functions_[i];
                return Status::Ok;
            }
        }
        return Status::NotFound;
    }

    [[nodiscard]] std::size_t function_count() const noexcept {
        return function_count_;
    }

private:
    SoftMmio&         mem_;
    DeviceGrant       grant_;
    const Descriptor* desc_{};
    PciFunction       functions_[kMaxFunctions]{};
    std::size_t       function_count_{0};
    bool              running_{false};
};

} // namespace pbsd::uda
