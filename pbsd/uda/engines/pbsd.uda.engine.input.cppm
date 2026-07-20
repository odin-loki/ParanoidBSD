module;
#include <concepts>
#include <cstdint>

export module pbsd.uda.engine.input;

export import pbsd.uda.interp;
import pbsd.core;

export namespace pbsd::uda {

struct HidReport {
    static constexpr std::size_t kMaxBytes = 8;
    std::uint8_t bytes[kMaxBytes]{};
    std::uint8_t length{};
};

struct HdaStreamDesc {
    std::uint8_t  stream_id{};
    std::uint16_t buffer_bytes{};
    std::uint32_t format{};
};

class InputEngine {
public:
    static constexpr std::size_t kMaxReports = 16;

    [[nodiscard]] static constexpr DeviceClass device_class() noexcept {
        return DeviceClass::Input;
    }

    explicit InputEngine(SoftMmio& mem, DeviceGrant grant = {}) noexcept
        : mem_(mem), grant_(grant.mmio_valid() ? grant
                                               : DeviceGrant{{0, SoftMmio::kSize}, {}}) {}

    [[nodiscard]] Status probe(const Descriptor& d) noexcept {
        if (validate_descriptor(d) != Status::Ok) {
            return Status::Invalid;
        }
        if (d.device_class != DeviceClass::Input) {
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
        tx_pending_ = 0;
        rx_pending_ = 0;
        report_count_ = 0;
        hda_pending_ = 0;
        BytecodeInterpreter interp{grant_, mem_};
        return interp.run(desc_->reset_sequence);
    }

    [[nodiscard]] DeviceGrant grant() const noexcept { return grant_; }

    [[nodiscard]] Status submit_report(HidReport const& report) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (report.length == 0 || report.length > HidReport::kMaxBytes) {
            return Status::Invalid;
        }
        if (report_count_ >= kMaxReports) {
            return Status::Busy;
        }
        reports_[report_count_++] = report;
        return Status::Ok;
    }

    [[nodiscard]] Status poll_report(HidReport* out) noexcept {
        if (!running_ || report_count_ == 0) {
            return Status::NotFound;
        }
        if (out) {
            *out = reports_[0];
        }
        for (std::size_t i = 1; i < report_count_; ++i) {
            reports_[i - 1] = reports_[i];
        }
        --report_count_;
        return Status::Ok;
    }

    [[nodiscard]] Status submit_char(std::uint8_t byte) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (tx_pending_ >= kMaxReports) {
            return Status::Busy;
        }
        tx_queue_[tx_pending_++] = byte;
        return Status::Ok;
    }

    [[nodiscard]] Status poll_char(std::uint8_t* out) noexcept {
        if (!running_ || rx_pending_ == 0) {
            return Status::NotFound;
        }
        if (out) {
            *out = rx_queue_[0];
        }
        for (std::size_t i = 1; i < rx_pending_; ++i) {
            rx_queue_[i - 1] = rx_queue_[i];
        }
        --rx_pending_;
        return Status::Ok;
    }

    [[nodiscard]] Status inject_char(std::uint8_t byte) noexcept {
        if (!running_ || rx_pending_ >= kMaxReports) {
            return Status::Busy;
        }
        rx_queue_[rx_pending_++] = byte;
        return Status::Ok;
    }

    [[nodiscard]] Status submit_hda_stream(HdaStreamDesc desc) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (desc.buffer_bytes == 0 || desc.stream_id > 15) {
            return Status::Invalid;
        }
        if (hda_pending_ >= kMaxReports) {
            return Status::Busy;
        }
        hda_queue_[hda_pending_++] = desc;
        return Status::Ok;
    }

    [[nodiscard]] Status complete_hda_stream(HdaStreamDesc* out) noexcept {
        if (!running_ || hda_pending_ == 0) {
            return Status::NotFound;
        }
        if (out) {
            *out = hda_queue_[0];
        }
        for (std::size_t i = 1; i < hda_pending_; ++i) {
            hda_queue_[i - 1] = hda_queue_[i];
        }
        --hda_pending_;
        return Status::Ok;
    }

    [[nodiscard]] Status submit_report_batch(HidReport const* reports,
                                             std::size_t count) noexcept {
        if (!running_ || reports == nullptr || count == 0) {
            return Status::Invalid;
        }
        if (report_count_ + count > kMaxReports) {
            return Status::Busy;
        }
        for (std::size_t i = 0; i < count; ++i) {
            if (reports[i].length == 0 || reports[i].length > HidReport::kMaxBytes) {
                return Status::Invalid;
            }
            reports_[report_count_++] = reports[i];
        }
        return Status::Ok;
    }

    [[nodiscard]] std::uint8_t modifier_state() const noexcept {
        return modifier_state_;
    }

    [[nodiscard]] Status set_modifier(std::uint8_t mask, bool down) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (down) {
            modifier_state_ |= mask;
        } else {
            modifier_state_ = static_cast<std::uint8_t>(modifier_state_ & ~mask);
        }
        return Status::Ok;
    }

private:
    SoftMmio&         mem_;
    DeviceGrant       grant_;
    const Descriptor* desc_{};
    HidReport         reports_[kMaxReports]{};
    std::uint8_t      tx_queue_[kMaxReports]{};
    std::uint8_t      rx_queue_[kMaxReports]{};
    std::size_t       report_count_{0};
    HdaStreamDesc     hda_queue_[kMaxReports]{};
    std::size_t       tx_pending_{0};
    std::size_t       rx_pending_{0};
    std::size_t       hda_pending_{0};
    std::uint8_t      modifier_state_{0};
    bool              running_{false};
};

} // namespace pbsd::uda
