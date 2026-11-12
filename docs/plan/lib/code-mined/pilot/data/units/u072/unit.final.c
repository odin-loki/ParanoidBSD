#include "prelude.h"
#undef _XE_USERPTR_H_
#define _XE_USERPTR_H_
/*@ requires true; ensures \result == 0; */
static inline int xe_vm_userptr_check_repin(struct xe_vm *vm) { return 0; }
