#include <sys/queue.h>
struct foo {
  int x;
  LIST_ENTRY(foo) entries;
};
LIST_HEAD(foolist, foo) head;
void f(struct foo *e) {
  LIST_INIT(&head);
  LIST_INSERT_HEAD(&head, e, entries);
  LIST_FOREACH(e, &head, entries) {
    (void)e->x;
  }
  LIST_REMOVE(e, entries);
}
