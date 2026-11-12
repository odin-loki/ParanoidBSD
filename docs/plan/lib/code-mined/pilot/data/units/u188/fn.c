// Intrusive singly-linked list with size(), push_back(), push_front()
// pop_front(), append_front() and append_back().
// This class should be a POD (so that it can be put into TLS)
// and an object with all zero fields should represent a valid empty list.
// This class does not have a CTOR, so clear() should be called on all
// non-zero-initialized objects before using.
template<class Item>
struct IntrusiveList {
  friend class Iterator;

  void clear() {
    first_ = last_ = nullptr;
    size_ = 0;
  }
