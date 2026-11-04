#pragma once
// PBSD staged intrusive container stubs — replace queue.h / tree.h macros.
#include <cstddef>

namespace pbsd::intrusive {
template <class T>
struct list_node {
  T* next{nullptr};
  T* prev{nullptr};
};
template <class T>
struct list_head {
  T* first{nullptr};
};
template <class T>
struct tailq_node {
  T* next{nullptr};
  T** prev{nullptr};
};
template <class T>
struct tailq_head {
  T* first{nullptr};
  T** last{nullptr};
};
template <class T>
struct slist_node {
  T* next{nullptr};
};
template <class T>
struct slist_head {
  T* first{nullptr};
};
template <class T>
struct stailq_head {
  T* first{nullptr};
  T** last{nullptr};
};
template <class T, class Key>
struct rb_node {
  T* left{};
  T* right{};
  T* parent{};
};

template <class Head>
inline void list_init(Head* h) {
  h->first = nullptr;
}
template <class Head>
inline void slist_init(Head* h) {
  h->first = nullptr;
}
template <class Head>
inline void tailq_init(Head* h) {
  h->first = nullptr;
  h->last = &h->first;
}
template <class Head>
inline void stailq_init(Head* h) {
  h->first = nullptr;
  h->last = &h->first;
}
}  // namespace pbsd::intrusive
