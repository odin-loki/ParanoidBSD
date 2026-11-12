#include "prelude.h"
#undef _DRM_P_H_
#define _DRM_P_H_
#undef DRM_DEBUG_DEFAULT_ON
#define DRM_DEBUG_DEFAULT_ON 1
#undef DRM_DEBUGBITS_DEBUG
#define	DRM_DEBUGBITS_DEBUG		0x1
#undef DRM_DEBUGBITS_KMS
#define	DRM_DEBUGBITS_KMS		0x2
#undef DRM_DEBUGBITS_FAILED_IOCTL
#define	DRM_DEBUGBITS_FAILED_IOCTL	0x4
#undef DRM_LINUX
#define DRM_LINUX 0
#undef DRIVER_USE_AGP
#define DRIVER_USE_AGP     0x1
#undef DRIVER_REQUIRE_AGP
#define DRIVER_REQUIRE_AGP 0x2
#undef DRIVER_USE_MTRR
#define DRIVER_USE_MTRR    0x4
#undef DRIVER_PCI_DMA
#define DRIVER_PCI_DMA     0x8
#undef DRIVER_SG
#define DRIVER_SG          0x10
#undef DRIVER_HAVE_DMA
#define DRIVER_HAVE_DMA    0x20
#undef DRIVER_HAVE_IRQ
#define DRIVER_HAVE_IRQ    0x40
#undef DRIVER_IRQ_SHARED
#define DRIVER_IRQ_SHARED  0x80
#undef DRIVER_IRQ_VBL
#define DRIVER_IRQ_VBL     0x100
#undef DRIVER_DMA_QUEUE
#define DRIVER_DMA_QUEUE   0x200
#undef DRIVER_FB_DMA
#define DRIVER_FB_DMA      0x400
#undef DRIVER_IRQ_VBL2
#define DRIVER_IRQ_VBL2    0x800
#undef DRIVER_GEM
#define DRIVER_GEM         0x1000
#undef DRIVER_MODESET
#define DRIVER_MODESET     0x2000
#undef DRIVER_PRIME
#define DRIVER_PRIME       0x4000
#undef DRIVER_BUS_PCI
#define DRIVER_BUS_PCI 0x1
#undef DRIVER_BUS_PLATFORM
#define DRIVER_BUS_PLATFORM 0x2
#undef DRIVER_BUS_USB
#define DRIVER_BUS_USB 0x3
#undef DRM_DEBUG_CODE
#define DRM_DEBUG_CODE 2	  /**< Include debugging code if > 1, then
#undef DRM_MAGIC_HASH_ORDER
#define DRM_MAGIC_HASH_ORDER  4  /**< Size of key hash table. Must be power of 2. */
#undef DRM_KERNEL_CONTEXT
#define DRM_KERNEL_CONTEXT    0	 /**< Change drm_resctx if changed */
#undef DRM_RESERVED_CONTEXTS
#define DRM_RESERVED_CONTEXTS 1	 /**< Change drm_resctx if changed */
#undef DRM_LOOPING_LIMIT
#define DRM_LOOPING_LIMIT     5000000
#undef DRM_TIME_SLICE
#define DRM_TIME_SLICE	      (HZ/20)  /**< Time slice for GLXContexts */
#undef DRM_LOCK_SLICE
#define DRM_LOCK_SLICE	      1	/**< Time slice for lock, in jiffies */
#undef DRM_FLAG_DEBUG
#define DRM_FLAG_DEBUG	  0x01
#undef DRM_MAX_CTXBITMAP
#define DRM_MAX_CTXBITMAP (PAGE_SIZE * 8)
#undef DRM_MAP_HASH_OFFSET
#define DRM_MAP_HASH_OFFSET 0x10000000
#undef DRM_ERROR
#define DRM_ERROR(fmt, ...)  	printf("error: [" DRM_NAME ":pid%d:%s] *ERROR* " fmt,		 	    DRM_CURRENTPID, __func__ , ##__VA_ARGS__)
#undef DRM_WARNING
#define DRM_WARNING(fmt, ...)  printf("warning: [" DRM_NAME "] " fmt , ##__VA_ARGS__)
#undef DRM_INFO
#define DRM_INFO(fmt, ...)  printf("info: [" DRM_NAME "] " fmt , ##__VA_ARGS__)
#undef DRM_DEBUG
#define DRM_DEBUG(fmt, ...) do {					 	if ((drm_debug & DRM_DEBUGBITS_DEBUG) != 0)			 		printf("[" DRM_NAME ":pid%d:%s] " fmt, DRM_CURRENTPID,	 			__func__ , ##__VA_ARGS__);			 } while (0)
#undef DRM_DEBUG_DRIVER
#define DRM_DEBUG_DRIVER(fmt, ...) do {					 	if ((drm_debug & DRM_DEBUGBITS_KMS) != 0)			 		printf("[" DRM_NAME ":KMS:pid%d:%s] " fmt, DRM_CURRENTPID, 			__func__ , ##__VA_ARGS__);			 } while (0)
#undef DRM_DEBUG_KMS
#define DRM_DEBUG_KMS(fmt, ...) do {					 	if ((drm_debug & DRM_DEBUGBITS_KMS) != 0)			 		printf("[" DRM_NAME ":KMS:pid%d:%s] " fmt, DRM_CURRENTPID, 			__func__ , ##__VA_ARGS__);			 } while (0)
#undef DRM_LOG
#define DRM_LOG(fmt, ...) do {						 	if ((drm_debug & DRM_DEBUGBITS_KMS) != 0)			 		printf("[" DRM_NAME "]:pid%d:%s]" fmt, DRM_CURRENTPID,	 			__func__ , ##__VA_ARGS__);			 } while (0)
#undef DRM_LOG_KMS
#define DRM_LOG_KMS(fmt, ...) do {					 	if ((drm_debug & DRM_DEBUGBITS_KMS) != 0)			 		printf("[" DRM_NAME "]:KMS:pid%d:%s]" fmt, DRM_CURRENTPID, 			__func__ , ##__VA_ARGS__);			 } while (0)
#undef DRM_LOG_MODE
#define DRM_LOG_MODE(fmt, ...) do {					 	if ((drm_debug & DRM_DEBUGBITS_KMS) != 0)			 		printf("[" DRM_NAME "]:pid%d:%s]" fmt, DRM_CURRENTPID,	 			__func__ , ##__VA_ARGS__);			 } while (0)
#undef DRM_LOG_DRIVER
#define DRM_LOG_DRIVER(fmt, ...) do {					 	if ((drm_debug & DRM_DEBUGBITS_KMS) != 0)			 		printf("[" DRM_NAME "]:KMS:pid%d:%s]" fmt, DRM_CURRENTPID, 			__func__ , ##__VA_ARGS__);			 } while (0)
#undef DRM_ARRAY_SIZE
#define DRM_ARRAY_SIZE(x) ARRAY_SIZE(x)
#undef DRM_LEFTCOUNT
#define DRM_LEFTCOUNT(x) (((x)->rp + (x)->count - (x)->wp) % ((x)->count + 1))
#undef DRM_BUFCOUNT
#define DRM_BUFCOUNT(x) ((x)->count - DRM_LEFTCOUNT(x))
#undef DRM_IF_VERSION
#define DRM_IF_VERSION(maj, min) (maj << 16 | min)
#undef LOCK_TEST_WITH_RETURN
#define LOCK_TEST_WITH_RETURN( dev, _file_priv )				 do {										 	if (!_DRM_LOCK_IS_HELD(_file_priv->master->lock.hw_lock->lock) ||	 	    _file_priv->master->lock.file_priv != _file_priv)	{		 		DRM_ERROR( "%s called without lock held, held  %d owner %p %p\n", 			   __func__, _DRM_LOCK_IS_HELD(_file_priv->master->lock.hw_lock->lock), 			   _file_priv->master->lock.file_priv, _file_priv);	 		return -EINVAL;							 	}									 } while (0)
#undef DRM_IOCTL_NR
#define DRM_IOCTL_NR(n)                ((n) & 0xff)
#undef DRM_MAJOR
#define DRM_MAJOR       226
#undef DRM_AUTH
#define DRM_AUTH	0x1
#undef DRM_MASTER
#define	DRM_MASTER	0x2
#undef DRM_ROOT_ONLY
#define DRM_ROOT_ONLY	0x4
#undef DRM_CONTROL_ALLOW
#define DRM_CONTROL_ALLOW 0x8
#undef DRM_UNLOCKED
#define DRM_UNLOCKED	0x10
#undef DRM_IOCTL_DEF
#define DRM_IOCTL_DEF(ioctl, _func, _flags)  	[DRM_IOCTL_NR(ioctl)] = {.cmd = ioctl, .func = _func, .flags = _flags, .cmd_drv = 0}
#undef DRM_IOCTL_DEF_DRV
#define DRM_IOCTL_DEF_DRV(ioctl, _func, _flags)			 	[DRM_IOCTL_NR(DRM_##ioctl)] = {.cmd = DRM_##ioctl, .func = _func, .flags = _flags, .cmd_drv = DRM_IOCTL_##ioctl}
#undef DRM_MAP_HANDLE_BITS
#define DRM_MAP_HANDLE_BITS	(sizeof(void *) == 4 ? 4 : 24)
#undef DRM_MAP_HANDLE_SHIFT
#define DRM_MAP_HANDLE_SHIFT	(sizeof(void *) * 8 - DRM_MAP_HANDLE_BITS)
#undef DRM_ATI_GART_MAIN
#define DRM_ATI_GART_MAIN 1
#undef DRM_ATI_GART_FB
#define DRM_ATI_GART_FB   2
#undef DRM_ATI_GART_PCI
#define DRM_ATI_GART_PCI 1
#undef DRM_ATI_GART_PCIE
#define DRM_ATI_GART_PCIE 2
#undef DRM_ATI_GART_IGP
#define DRM_ATI_GART_IGP 3
#undef DRM_VBLANKTIME_RBSIZE
#define DRM_VBLANKTIME_RBSIZE 2
#undef DRM_CALLED_FROM_VBLIRQ
#define DRM_CALLED_FROM_VBLIRQ 1
#undef DRM_VBLANKTIME_SCANOUTPOS_METHOD
#define DRM_VBLANKTIME_SCANOUTPOS_METHOD (1 << 0)
#undef DRM_VBLANKTIME_INVBL
#define DRM_VBLANKTIME_INVBL             (1 << 1)
#undef DRM_SCANOUTPOS_VALID
#define DRM_SCANOUTPOS_VALID        (1 << 0)
#undef DRM_SCANOUTPOS_INVBL
#define DRM_SCANOUTPOS_INVBL        (1 << 1)
#undef DRM_SCANOUTPOS_ACCURATE
#define DRM_SCANOUTPOS_ACCURATE     (1 << 2)
#undef DRM_MINOR_UNASSIGNED
#define DRM_MINOR_UNASSIGNED 0
#undef DRM_MINOR_LEGACY
#define DRM_MINOR_LEGACY 1
#undef DRM_MINOR_CONTROL
#define DRM_MINOR_CONTROL 2
#undef DRM_MINOR_RENDER
#define DRM_MINOR_RENDER 3
#undef DRM_MAX_PCI_RESOURCE
#define	DRM_MAX_PCI_RESOURCE	6
#undef DRM_SWITCH_POWER_ON
#define DRM_SWITCH_POWER_ON 0
#undef DRM_SWITCH_POWER_OFF
#define DRM_SWITCH_POWER_OFF 1
#undef DRM_SWITCH_POWER_CHANGING
#define DRM_SWITCH_POWER_CHANGING 2
#undef drm_core_has_AGP
#define drm_core_has_AGP(dev) (0)
#undef drm_core_has_MTRR
#define drm_core_has_MTRR(dev) (0)
#undef DRM_PCIE_SPEED_25
#define DRM_PCIE_SPEED_25 1
#undef DRM_PCIE_SPEED_50
#define DRM_PCIE_SPEED_50 2
#undef DRM_PCIE_SPEED_80
#define DRM_PCIE_SPEED_80 4
#undef drm_can_sleep
#define	drm_can_sleep()	(DRM_HZ & 1)
#undef DRM_GEM_MAPPING_MASK
#define	DRM_GEM_MAPPING_MASK	(3ULL << 62)
#undef DRM_GEM_MAPPING_KEY
#define	DRM_GEM_MAPPING_KEY	(2ULL << 62) 
#undef DRM_GEM_MAX_IDX
#define	DRM_GEM_MAX_IDX		0x3fffff
#undef DRM_GEM_MAPPING_IDX
#define	DRM_GEM_MAPPING_IDX(o)	(((o) >> 40) & DRM_GEM_MAX_IDX)
#undef DRM_GEM_MAPPING_OFF
#define	DRM_GEM_MAPPING_OFF(i)	(((uint64_t)(i)) << 40)
#undef DRM_GEM_MAPPING_MAPOFF
#define	DRM_GEM_MAPPING_MAPOFF(o)      ((o) & ~(DRM_GEM_MAPPING_OFF(DRM_GEM_MAX_IDX) | DRM_GEM_MAPPING_KEY))
#undef DRM_DEV_MODE
#define DRM_DEV_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)
#undef DRM_DEV_UID
#define DRM_DEV_UID	UID_ROOT
#undef DRM_DEV_GID
#define DRM_DEV_GID	GID_VIDEO
#undef DRM_WAKEUP
#define DRM_WAKEUP(w)		wakeup((void *)w)
#undef DRM_WAKEUP_INT
#define DRM_WAKEUP_INT(w)	wakeup(w)
#undef DRM_INIT_WAITQUEUE
#define DRM_INIT_WAITQUEUE(queue) do {(void)(queue);} while (0)
#undef DRM_CURPROC
#define DRM_CURPROC		curthread
#undef DRM_STRUCTPROC
#define DRM_STRUCTPROC		struct thread
#undef DRM_SPINTYPE
#define DRM_SPINTYPE		struct mtx
#undef DRM_SPININIT
#define DRM_SPININIT(l,name)	mtx_init(l, name, NULL, MTX_DEF)
#undef DRM_SPINUNINIT
#define DRM_SPINUNINIT(l)	mtx_destroy(l)
#undef DRM_SPINLOCK
#define DRM_SPINLOCK(l)		mtx_lock(l)
#undef DRM_SPINUNLOCK
#define DRM_SPINUNLOCK(u)	mtx_unlock(u)
#undef DRM_SPINLOCK_IRQSAVE
#define DRM_SPINLOCK_IRQSAVE(l, irqflags) do {		 	mtx_lock(l);					 	(void)irqflags;					 } while (0)
#undef DRM_SPINUNLOCK_IRQRESTORE
#define DRM_SPINUNLOCK_IRQRESTORE(u, irqflags) mtx_unlock(u)
#undef DRM_SPINLOCK_ASSERT
#define DRM_SPINLOCK_ASSERT(l)	mtx_assert(l, MA_OWNED)
#undef DRM_LOCK_SLEEP
#define	DRM_LOCK_SLEEP(dev, chan, flags, msg, timeout)			     (sx_sleep((chan), &(dev)->dev_struct_lock, (flags), (msg), (timeout)))
#undef DRM_SYSCTL_HANDLER_ARGS
#define DRM_SYSCTL_HANDLER_ARGS	(SYSCTL_HANDLER_ARGS)
#undef DRM_VERIFYAREA_READ
#define DRM_VERIFYAREA_READ( uaddr, size )		 	(!useracc(__DECONST(caddr_t, uaddr), size, VM_PROT_READ))
#undef DRM_COPY_TO_USER
#define DRM_COPY_TO_USER(user, kern, size)  	copyout(kern, user, size)
#undef DRM_COPY_FROM_USER
#define DRM_COPY_FROM_USER(kern, user, size)  	copyin(user, kern, size)
#undef DRM_COPY_FROM_USER_UNCHECKED
#define DRM_COPY_FROM_USER_UNCHECKED(arg1, arg2, arg3) 	 	copyin(arg2, arg1, arg3)
#undef DRM_COPY_TO_USER_UNCHECKED
#define DRM_COPY_TO_USER_UNCHECKED(arg1, arg2, arg3)	 	copyout(arg2, arg1, arg3)
#undef DRM_GET_USER_UNCHECKED
#define DRM_GET_USER_UNCHECKED(val, uaddr)		 	((val) = fuword32(uaddr), 0)
#undef DRM_GET_PRIV_SAREA
#define DRM_GET_PRIV_SAREA(_dev, _ctx, _map) do {	 	(_map) = (_dev)->context_sareas[_ctx];		 } while(0)
#undef DRM_WAIT_ON
#define DRM_WAIT_ON( ret, queue, timeout, condition )		 for ( ret = 0 ; !ret && !(condition) ; ) {			 	DRM_UNLOCK(dev);					 	mtx_lock(&dev->irq_lock);				 	if (!(condition))					 	    ret = -mtx_sleep(&(queue), &dev->irq_lock, 		 		PCATCH, "drmwtq", (timeout));			 	    if (ret == -ERESTART)				 	        ret = -ERESTARTSYS;				 	mtx_unlock(&dev->irq_lock);				 	DRM_LOCK(dev);						 }
#undef dev_err
#define	dev_err(dev, fmt, ...)						 	device_printf((dev), "error: " fmt, ## __VA_ARGS__)
#undef dev_warn
#define	dev_warn(dev, fmt, ...)						 	device_printf((dev), "warning: " fmt, ## __VA_ARGS__)
#undef dev_info
#define	dev_info(dev, fmt, ...)						 	device_printf((dev), "info: " fmt, ## __VA_ARGS__)
#undef dev_dbg
#define	dev_dbg(dev, fmt, ...) do {					 	if ((drm_debug& DRM_DEBUGBITS_KMS) != 0) {			 		device_printf((dev), "debug: " fmt, ## __VA_ARGS__);	 	}								 } while (0)
#undef DMA_BIT_MASK
#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : (1ULL<<(n)) - 1)
#undef upper_32_bits
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#undef DMI_MATCH
#define	DMI_MATCH(a, b) {(a), (b)}
/*@ requires true; ensures \result == -ENODEV; */
static inline int drm_agp_info(struct drm_device *dev,
			       struct drm_agp_info *info)
{
	return -ENODEV;
}
