module;
#include <concepts>
#include <cstdint>

export module pbsd.uda.engine.display;

export import pbsd.uda.interp;
import pbsd.core;

export namespace pbsd::uda {

enum class DisplayCmd : std::uint32_t {
    GetDisplayInfo   = 0x0100,
    ResourceCreate2d = 0x0101,
    SetScanout       = 0x0103,
    ResourceFlush    = 0x0104,
    TransferToHost2d = 0x0105,
    GetEdid          = 0x010A,
};

struct DisplayRequest {
    DisplayCmd    cmd{};
    std::uint32_t resource_id{};
    std::uint32_t scanout_id{};
    std::uint32_t width{};
    std::uint32_t height{};
};

class DisplayEngine {
public:
    static constexpr std::size_t kMaxPending = 8;

    [[nodiscard]] static constexpr DeviceClass device_class() noexcept {
        return DeviceClass::Display;
    }

    explicit DisplayEngine(SoftMmio& mem, DeviceGrant grant = {}) noexcept
        : mem_(mem), grant_(grant.mmio_valid() ? grant
                                               : DeviceGrant{{0, SoftMmio::kSize}, {}}) {}

    [[nodiscard]] Status probe(const Descriptor& d) noexcept {
        if (validate_descriptor(d) != Status::Ok) {
            return Status::Invalid;
        }
        if (d.device_class != DeviceClass::Display) {
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
        pending_count_ = 0;
        BytecodeInterpreter interp{grant_, mem_};
        return interp.run(desc_->reset_sequence);
    }

    [[nodiscard]] DeviceGrant grant() const noexcept { return grant_; }

    [[nodiscard]] Status submit_scanout(std::uint32_t scanout_id,
                                        std::uint32_t resource_id,
                                        std::uint32_t width,
                                        std::uint32_t height) noexcept {
        return submit({DisplayCmd::SetScanout, resource_id, scanout_id, width, height});
    }

    [[nodiscard]] Status submit_flush(std::uint32_t resource_id) noexcept {
        return submit({DisplayCmd::ResourceFlush, resource_id, 0, 0, 0});
    }

    [[nodiscard]] Status submit_get_edid(std::uint32_t scanout_id) noexcept {
        return submit({DisplayCmd::GetEdid, 0, scanout_id, 0, 0});
    }

    [[nodiscard]] Status submit_create2d(std::uint32_t resource_id,
                                         std::uint32_t width,
                                         std::uint32_t height) noexcept {
        if (width == 0 || height == 0) {
            return Status::Invalid;
        }
        return submit({DisplayCmd::ResourceCreate2d, resource_id, 0, width, height});
    }

    [[nodiscard]] Status submit_transfer(std::uint32_t resource_id,
                                         std::uint32_t width,
                                         std::uint32_t height) noexcept {
        return submit({DisplayCmd::TransferToHost2d, resource_id, 0, width, height});
    }

    [[nodiscard]] Status submit(DisplayRequest req) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (pending_count_ >= kMaxPending) {
            return Status::Busy;
        }
        pending_[pending_count_++] = req;
        return Status::Ok;
    }

    [[nodiscard]] Status complete_one(DisplayRequest* out) noexcept {
        if (!running_ || pending_count_ == 0) {
            return Status::NotFound;
        }
        if (out) {
            *out = pending_[0];
        }
        for (std::size_t i = 1; i < pending_count_; ++i) {
            pending_[i - 1] = pending_[i];
        }
        --pending_count_;
        return Status::Ok;
    }

    [[nodiscard]] std::size_t pending_count() const noexcept {
        return pending_count_;
    }

private:
    SoftMmio&         mem_;
    DeviceGrant       grant_;
    const Descriptor* desc_{};
    DisplayRequest    pending_[kMaxPending]{};
    std::size_t       pending_count_{0};
    bool              running_{false};
};

} // namespace pbsd::uda
