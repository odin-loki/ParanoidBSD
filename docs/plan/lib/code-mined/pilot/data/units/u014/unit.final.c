#include "prelude.h"
#undef WANT_NATIVE_PCI_GET_SLOT
#define	WANT_NATIVE_PCI_GET_SLOT
#undef DMA_PRIV_LOCK
#define	DMA_PRIV_LOCK(priv) mtx_lock(&(priv)->lock)
#undef DMA_PRIV_UNLOCK
#define	DMA_PRIV_UNLOCK(priv) mtx_unlock(&(priv)->lock)
#undef DMA_POOL_LOCK
#define	DMA_POOL_LOCK(pool) mtx_lock(&(pool)->pool_lock)
#undef DMA_POOL_UNLOCK
#define	DMA_POOL_UNLOCK(pool) mtx_unlock(&(pool)->pool_lock)
#undef PCIR_MAX_BAR_0
#define PCIR_MAX_BAR_0		5
/*@ requires true; ensures \result == (0 <= bar && bar <= PCIR_MAX_BAR_0); */
static bool
lkpi_pci_bar_id_valid(int bar)
{
	if (bar < 0 || bar > PCIR_MAX_BAR_0)
		return (false);

	return (true);
}
