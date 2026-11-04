/* Dual-link proof — C side calls C++ exports; exports C symbols for C++ init. */
#include "pbsd_cxx_hello.h"

#ifdef _KERNEL
#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#endif

const char*
pbsd_cxx_hello_c_tag(void)
{
    return "pbsd_cxx_hello/c";
}

int
pbsd_cxx_hello_c_smoke(void)
{
    if (pbsd_cxx_hello_init() != 0) {
        return 1;
    }
    if (pbsd_cxx_hello_version() == NULL) {
        return 2;
    }
    return pbsd_cxx_hello_fini();
}

#ifdef _KERNEL
static int
pbsd_cxx_hello_modevent(module_t mod, int what, void *arg)
{
    int error;

    (void)mod;
    (void)arg;

    switch (what) {
    case MOD_LOAD:
        error = pbsd_cxx_hello_c_smoke();
        if (error != 0) {
            printf("pbsd_cxx_hello: smoke failed (%d)\n", error);
            return (ENXIO);
        }
        printf("pbsd_cxx_hello: loaded (%s)\n", pbsd_cxx_hello_version());
        return (0);
    case MOD_UNLOAD:
        return (pbsd_cxx_hello_fini());
    default:
        return (EOPNOTSUPP);
    }
}

static moduledata_t pbsd_cxx_hello_mod = {
    "pbsd_cxx_hello",
    pbsd_cxx_hello_modevent,
    NULL
};

DECLARE_MODULE(pbsd_cxx_hello, pbsd_cxx_hello_mod, SI_SUB_DRIVERS, SI_ORDER_ANY);
#endif

#ifdef PBSD_CXX_HELLO_HOST_SMOKE
int
main(void)
{
    return (pbsd_cxx_hello_c_smoke());
}
#endif
