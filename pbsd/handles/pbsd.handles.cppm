export module pbsd.handles;

export import pbsd.core;
export import pbsd.rights;
export import pbsd.cap.lineage;

export namespace pbsd {

template<typename T>
concept KernelObject = requires(T* p) {
    { T::release(p) } noexcept;
};

template<KernelObject T>
class BorrowedHandle;

template<KernelObject T>
class SharedHandle;

template<KernelObject T>
class [[nodiscard]] UniqueHandle {
public:
    UniqueHandle() noexcept = default;

    explicit UniqueHandle(T* ptr, CapabilityRights rights, LineageId lineage) noexcept
        : ptr_(ptr), rights_(rights), lineage_(lineage) {}

    UniqueHandle(UniqueHandle&& o) noexcept
        : ptr_(o.ptr_), rights_(o.rights_), lineage_(o.lineage_) {
        o.clear();
    }

    UniqueHandle& operator=(UniqueHandle&& o) noexcept {
        if (this != &o) {
            reset();
            ptr_ = o.ptr_;
            rights_ = o.rights_;
            lineage_ = o.lineage_;
            o.clear();
        }
        return *this;
    }

    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;

    ~UniqueHandle() noexcept { reset(); }

    [[nodiscard]] bool valid() const noexcept { return ptr_ != nullptr; }
    [[nodiscard]] CapabilityRights rights() const noexcept { return rights_; }
    [[nodiscard]] LineageId lineage() const noexcept { return lineage_; }

    [[nodiscard]] bool has_right(CapabilityRights r) const noexcept {
        return pbsd::has_right(rights_, r);
    }

    [[nodiscard]] bool is_alive(const LineageTree& tree) const noexcept {
        return valid() && tree.valid(lineage_);
    }

    [[nodiscard]] UniqueHandle narrow(CapabilityRights want) && noexcept {
        CapabilityRights n = narrow_rights(rights_, want);
        UniqueHandle h(ptr_, n, lineage_);
        clear();
        return h;
    }

    [[nodiscard]] Result<BorrowedHandle<T>> grant(CapabilityRights child_rights,
                                                  LineageTree& tree) const noexcept;

    [[nodiscard]] Result<BorrowedHandle<T>> duplicate(CapabilityRights dup_rights,
                                                      LineageTree& tree) const noexcept;

    [[nodiscard]] BorrowedHandle<T> borrow(CapabilityRights want) const noexcept;

    void reset() noexcept {
        if (ptr_) {
            T::release(ptr_);
            clear();
        }
    }

    /// Kernel-internal access only — no public get().
    [[nodiscard]] T* peek() const noexcept { return ptr_; }

    template<KernelObject U>
    friend Result<SharedHandle<U>> share(UniqueHandle<U>&& owner, unsigned& ref_storage) noexcept;

private:
    friend class BorrowedHandle<T>;
    friend class SharedHandle<T>;

    T* ptr_ = nullptr;
    CapabilityRights rights_ = CapabilityRights::None;
    LineageId lineage_ = kInvalidLineage;

    void clear() noexcept {
        ptr_ = nullptr;
        rights_ = CapabilityRights::None;
        lineage_ = kInvalidLineage;
    }
};

template<KernelObject T>
class BorrowedHandle {
public:
    BorrowedHandle() noexcept = default;

    BorrowedHandle(T* ptr, CapabilityRights rights, LineageId lineage) noexcept
        : ptr_(ptr), rights_(rights), lineage_(lineage) {}

    [[nodiscard]] bool valid() const noexcept { return ptr_ != nullptr; }
    [[nodiscard]] CapabilityRights rights() const noexcept { return rights_; }
    [[nodiscard]] LineageId lineage() const noexcept { return lineage_; }

    [[nodiscard]] bool has_right(CapabilityRights r) const noexcept {
        return pbsd::has_right(rights_, r);
    }

    [[nodiscard]] bool is_alive(const LineageTree& tree) const noexcept {
        return valid() && tree.valid(lineage_);
    }

    [[nodiscard]] T* peek() const noexcept { return ptr_; }

private:
    T* ptr_ = nullptr;
    CapabilityRights rights_ = CapabilityRights::None;
    LineageId lineage_ = kInvalidLineage;
};

template<KernelObject T>
[[nodiscard]] BorrowedHandle<T> UniqueHandle<T>::borrow(CapabilityRights want) const noexcept {
    CapabilityRights n = narrow_rights(rights_, want);
    return BorrowedHandle<T>{ptr_, n, lineage_};
}

template<KernelObject T>
[[nodiscard]] Result<BorrowedHandle<T>>
UniqueHandle<T>::grant(CapabilityRights child_rights, LineageTree& tree) const noexcept {
    if (!valid() || !has_right(CapabilityRights::Grant)) {
        return result_err<BorrowedHandle<T>>(Status::Denied);
    }
    if (check_grant(rights_, child_rights) != Status::Ok) {
        return result_err<BorrowedHandle<T>>(Status::Denied);
    }
    LineageId child = tree.create_child(lineage_);
    if (child == kInvalidLineage) {
        return result_err<BorrowedHandle<T>>(Status::NoMemory);
    }
    return result_ok(BorrowedHandle<T>{ptr_, child_rights, child});
}

template<KernelObject T>
[[nodiscard]] Result<BorrowedHandle<T>>
UniqueHandle<T>::duplicate(CapabilityRights dup_rights, LineageTree& tree) const noexcept {
    if (!valid()) {
        return result_err<BorrowedHandle<T>>(Status::Invalid);
    }
    if (check_duplicate(rights_, dup_rights) != Status::Ok) {
        return result_err<BorrowedHandle<T>>(Status::Denied);
    }
    LineageId child = tree.create_child(lineage_);
    if (child == kInvalidLineage) {
        return result_err<BorrowedHandle<T>>(Status::NoMemory);
    }
    return result_ok(BorrowedHandle<T>{ptr_, dup_rights, child});
}

template<KernelObject T>
class SharedHandle {
public:
    SharedHandle() noexcept = default;

    SharedHandle(T* ptr, CapabilityRights rights, LineageId lineage, unsigned* ref) noexcept
        : ptr_(ptr), rights_(rights), lineage_(lineage), ref_(ref) {
        if (ref_) {
            ++(*ref_);
        }
    }

    SharedHandle(const SharedHandle& o) noexcept
        : ptr_(o.ptr_), rights_(o.rights_), lineage_(o.lineage_), ref_(o.ref_) {
        if (ref_) {
            ++(*ref_);
        }
    }

    SharedHandle& operator=(const SharedHandle& o) noexcept {
        if (this != &o) {
            drop();
            ptr_ = o.ptr_;
            rights_ = o.rights_;
            lineage_ = o.lineage_;
            ref_ = o.ref_;
            if (ref_) {
                ++(*ref_);
            }
        }
        return *this;
    }

    SharedHandle(SharedHandle&& o) noexcept
        : ptr_(o.ptr_), rights_(o.rights_), lineage_(o.lineage_), ref_(o.ref_) {
        o.ptr_ = nullptr;
        o.ref_ = nullptr;
        o.rights_ = CapabilityRights::None;
        o.lineage_ = kInvalidLineage;
    }

    ~SharedHandle() noexcept { drop(); }

    [[nodiscard]] bool valid() const noexcept { return ptr_ != nullptr; }
    [[nodiscard]] CapabilityRights rights() const noexcept { return rights_; }
    [[nodiscard]] LineageId lineage() const noexcept { return lineage_; }

    [[nodiscard]] bool has_right(CapabilityRights r) const noexcept {
        return pbsd::has_right(rights_, r);
    }

    [[nodiscard]] bool is_alive(const LineageTree& tree) const noexcept {
        return valid() && tree.valid(lineage_);
    }

    [[nodiscard]] T* peek() const noexcept { return ptr_; }

private:
    T* ptr_ = nullptr;
    CapabilityRights rights_ = CapabilityRights::None;
    LineageId lineage_ = kInvalidLineage;
    unsigned* ref_ = nullptr;

    void drop() noexcept {
        if (ref_ && --(*ref_) == 0) {
            T::release(ptr_);
        }
        ptr_ = nullptr;
        ref_ = nullptr;
        rights_ = CapabilityRights::None;
        lineage_ = kInvalidLineage;
    }
};

template<KernelObject T>
[[nodiscard]] Result<SharedHandle<T>> share(UniqueHandle<T>&& owner,
                                           unsigned& ref_storage) noexcept {
    if (!owner.valid()) {
        return result_err<SharedHandle<T>>(Status::Invalid);
    }
    ref_storage = 0;
    SharedHandle<T> shared{owner.peek(), owner.rights(), owner.lineage(), &ref_storage};
    owner.clear();
    return result_ok(util::move(shared));
}

struct VmObject {
    static void release(VmObject* p) noexcept { (void)p; }
};

using VmHandle = UniqueHandle<VmObject>;

} // namespace pbsd
