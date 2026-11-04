/* Dual-link C ABI — callable from C KPI / kmod glue (Wave 0). */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* C++ exports (defined in pbsd_cxx_hello.cpp) */
int pbsd_cxx_hello_init(void);
int pbsd_cxx_hello_fini(void);
const char* pbsd_cxx_hello_version(void);

/* C exports (defined in pbsd_cxx_hello.c) — called from C++ during init */
const char* pbsd_cxx_hello_c_tag(void);
int pbsd_cxx_hello_c_smoke(void);

#ifdef __cplusplus
}
#endif
