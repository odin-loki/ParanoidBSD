module;
#include <concepts>
#include <cstdint>

export module pbsd.uda.engine.block;

export import pbsd.uda.interp;
import pbsd.core;

export namespace pbsd::uda {

enum class BlockOp : std::uint32_t {
    Read  = 0,
    Write = 1,
    Flush = 4,
    Trim  = 9,
};

struct BlockRequest {
    BlockOp       op{};
    std::uint64_t sector{};
    std::uint32_t ioprio{};
    std::uint32_t sector_count{1};
};

class BlockEngine {
public:
    static constexpr std::size_t kMaxPending = 16;

    [[nodiscard]] static constexpr DeviceClass device_class() noexcept {
        return DeviceClass::Block;
    }

    explicit BlockEngine(SoftMmio& mem, DeviceGrant grant = {}) noexcept
        : mem_(mem), grant_(grant.mmio_valid() ? grant
                                               : DeviceGrant{{0, SoftMmio::kSize}, {}}) {}

    [[nodiscard]] Status probe(const Descriptor& d) noexcept {
        if (validate_descriptor(d) != Status::Ok) {
            return Status::Invalid;
        }
        if (d.device_class != DeviceClass::Block) {
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

    [[nodiscard]] Status submit_read(std::uint64_t sector,
                                     std::uint32_t count = 1) noexcept {
        return submit({BlockOp::Read, sector, 0, count});
    }

    [[nodiscard]] Status submit_write(std::uint64_t sector,
                                      std::uint32_t count = 1) noexcept {
        return submit({BlockOp::Write, sector, 0, count});
    }

    [[nodiscard]] Status submit_flush() noexcept {
        return submit({BlockOp::Flush, 0, 0, 0});
    }

    [[nodiscard]] Status submit_trim(std::uint64_t sector,
                                     std::uint32_t count = 1) noexcept {
        return submit({BlockOp::Trim, sector, 0, count});
    }

    [[nodiscard]] Status submit(BlockRequest req) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (pending_count_ >= kMaxPending) {
            return Status::Busy;
        }
        pending_[pending_count_++] = req;
        return Status::Ok;
    }

    [[nodiscard]] Status complete_one(BlockRequest* out) noexcept {
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

    [[nodiscard]] Status submit_nvme_io(BlockRequest req,
                                      std::uint8_t nvme_opc) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (pending_count_ >= kMaxPending) {
            return Status::Busy;
        }
        pending_[pending_count_++] = req;
        nvme_opc_queue_[pending_count_ - 1] = nvme_opc;
        return Status::Ok;
    }

    [[nodiscard]] Status complete_nvme_io(BlockRequest* req_out,
                                          std::uint8_t* opc_out) noexcept {
        if (!running_ || pending_count_ == 0) {
            return Status::NotFound;
        }
        if (req_out) {
            *req_out = pending_[0];
        }
        if (opc_out) {
            *opc_out = nvme_opc_queue_[0];
        }
        for (std::size_t i = 1; i < pending_count_; ++i) {
            pending_[i - 1] = pending_[i];
            nvme_opc_queue_[i - 1] = nvme_opc_queue_[i];
        }
        --pending_count_;
        return Status::Ok;
    }

    [[nodiscard]] std::size_t pending_count() const noexcept {
        return pending_count_;
    }

    [[nodiscard]] Status submit_ahci_fis(std::uint64_t sector,
                                         std::uint32_t count,
                                         BlockOp op) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (pending_count_ >= kMaxPending) {
            return Status::Busy;
        }
        pending_[pending_count_] = {op, sector, 0, count};
        ahci_sig_queue_[pending_count_] = 0xEB140000u;
        ++pending_count_;
        return Status::Ok;
    }

    [[nodiscard]] Status complete_ahci_fis(BlockRequest* req_out,
                                           std::uint32_t* sig_out) noexcept {
        if (!running_ || pending_count_ == 0) {
            return Status::NotFound;
        }
        if (req_out) {
            *req_out = pending_[0];
        }
        if (sig_out) {
            *sig_out = ahci_sig_queue_[0];
        }
        for (std::size_t i = 1; i < pending_count_; ++i) {
            pending_[i - 1] = pending_[i];
            ahci_sig_queue_[i - 1] = ahci_sig_queue_[i];
        }
        --pending_count_;
        return Status::Ok;
    }

    [[nodiscard]] Status submit_cam_io(BlockRequest req, std::uint8_t cam_status) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (pending_count_ >= kMaxPending) {
            return Status::Busy;
        }
        pending_[pending_count_] = req;
        cam_status_queue_[pending_count_] = cam_status;
        ++pending_count_;
        ++cam_submit_count_;
        return Status::Ok;
    }

    [[nodiscard]] Status complete_cam_io(BlockRequest* req_out,
                                           std::uint8_t* cam_status_out) noexcept {
        if (!running_ || pending_count_ == 0) {
            return Status::NotFound;
        }
        if (req_out) {
            *req_out = pending_[0];
        }
        if (cam_status_out) {
            *cam_status_out = cam_status_queue_[0];
        }
        for (std::size_t i = 1; i < pending_count_; ++i) {
            pending_[i - 1] = pending_[i];
            cam_status_queue_[i - 1] = cam_status_queue_[i];
        }
        --pending_count_;
        return Status::Ok;
    }

    [[nodiscard]] std::uint64_t cam_submit_count() const noexcept {
        return cam_submit_count_;
    }

    [[nodiscard]] Status submit_mpt_io(BlockRequest req, std::uint32_t doorbell) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (pending_count_ >= kMaxPending) {
            return Status::Busy;
        }
        pending_[pending_count_] = req;
        mpt_db_queue_[pending_count_] = doorbell;
        ++pending_count_;
        return Status::Ok;
    }

    [[nodiscard]] Status complete_mpt_io(BlockRequest* req_out,
                                       std::uint32_t* doorbell_out) noexcept {
        if (!running_ || pending_count_ == 0) {
            return Status::NotFound;
        }
        if (req_out) {
            *req_out = pending_[0];
        }
        if (doorbell_out) {
            *doorbell_out = mpt_db_queue_[0];
        }
        for (std::size_t i = 1; i < pending_count_; ++i) {
            pending_[i - 1] = pending_[i];
            mpt_db_queue_[i - 1] = mpt_db_queue_[i];
        }
        --pending_count_;
        return Status::Ok;
    }

private:
    SoftMmio&         mem_;
    DeviceGrant       grant_;
    const Descriptor* desc_{};
    BlockRequest      pending_[kMaxPending]{};
    std::uint8_t      nvme_opc_queue_[kMaxPending]{};
    std::uint32_t     ahci_sig_queue_[kMaxPending]{};
    std::uint8_t      cam_status_queue_[kMaxPending]{};
    std::uint32_t     mpt_db_queue_[kMaxPending]{};
    std::size_t       pending_count_{0};
    std::uint64_t     cam_submit_count_{0};
    bool              running_{false};
};

} // namespace pbsd::uda
