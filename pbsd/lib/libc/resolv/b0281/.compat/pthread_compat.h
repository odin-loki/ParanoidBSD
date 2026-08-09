#ifndef PBSD_B0281_PTHREAD_COMPAT_H
#define PBSD_B0281_PTHREAD_COMPAT_H
#define __bits_pthreadtypes_common_h 1
#define _BITS_PTHREADTYPES_COMMON_H 1
typedef struct { int id; } pthread_key_t;
typedef struct { int locked; } pthread_mutex_t;
#endif
