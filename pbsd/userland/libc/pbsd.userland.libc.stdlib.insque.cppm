module;

export module pbsd.userland.libc.stdlib.insque;

/// insque/remque from hbsd/src/lib/libc/stdlib/{insque,remque}.c
export namespace pbsd::userland::libc {

struct QueElem {
    QueElem* prev{nullptr};
    QueElem* next{nullptr};
};

inline void insque(QueElem& elem, QueElem* pred) noexcept {
    if (pred == nullptr) {
        elem.prev = nullptr;
        elem.next = nullptr;
        return;
    }
    QueElem* next = pred->next;
    if (next != nullptr) {
        next->prev = &elem;
    }
    pred->next = &elem;
    elem.prev = pred;
    elem.next = next;
}

inline void remque(QueElem& elem) noexcept {
    if (elem.prev != nullptr) {
        elem.prev->next = elem.next;
    }
    if (elem.next != nullptr) {
        elem.next->prev = elem.prev;
    }
}

} // namespace pbsd::userland::libc
