#include "prelude.h"
#undef _NET_NETMAP_KERN_H_
#define _NET_NETMAP_KERN_H_
#undef WITH_PTNETMAP
#define WITH_PTNETMAP
#undef WITH_SINK
#define WITH_SINK
#undef __user
#define __user
#undef NM_MTX_SPINLOCK
#define NM_MTX_SPINLOCK(m)	while (!sx_try_xlock(&(m))) ;
#undef MBUF_TXQ
#define MBUF_TXQ(m)	((m)->m_pkthdr.flowid)
#undef GEN_TX_MBUF_NA
#define	GEN_TX_MBUF_NA(m)	((struct netmap_adapter *)(m)->m_ext.ext_arg1)
#undef NM_ATOMIC_TEST_AND_SET
#define NM_ATOMIC_TEST_AND_SET(p)       (!atomic_cmpset_acq_int((p), 0, 1))
#undef NM_ATOMIC_CLEAR
#define NM_ATOMIC_CLEAR(p)              atomic_store_rel_int((p), 0)
#undef MBUF_REFCNT
#define MBUF_REFCNT(m)		((m)->m_ext.ext_count)
#undef SET_MBUF_REFCNT
#define SET_MBUF_REFCNT(m, x)   (m)->m_ext.ext_count = x
#undef MBUF_QUEUED
#define MBUF_QUEUED(m)		1
#undef NM_BNS_GET
#define NM_BNS_GET(b)
#undef NM_BNS_PUT
#define NM_BNS_PUT(b)
#undef DEV_NETMAP
#define DEV_NETMAP
#undef NETMAP_NDIS_LINKNAME_STRING
#define NETMAP_NDIS_LINKNAME_STRING             L"\\DosDevices\\NMAPNDIS"
#undef NETMAP_NDIS_NTDEVICE_STRING
#define NETMAP_NDIS_NTDEVICE_STRING             L"\\Device\\NMAPNDIS"
#undef NETMAP_KERNEL_XCHANGE_POINTERS
#define NETMAP_KERNEL_XCHANGE_POINTERS		_IO('i', 180)
#undef NETMAP_KERNEL_SEND_SHUTDOWN_SIGNAL
#define NETMAP_KERNEL_SEND_SHUTDOWN_SIGNAL	_IO_direct('i', 195)
#undef SYSBEGIN
#define SYSBEGIN(x)
#undef SYSEND
#define SYSEND
#undef NM_ACCESS_ONCE
#define NM_ACCESS_ONCE(x)	(*(volatile __typeof__(x) *)&(x))
#undef NMG_LOCK_T
#define	NMG_LOCK_T		NM_MTX_T
#undef NMG_LOCK_INIT
#define	NMG_LOCK_INIT()		NM_MTX_INIT(netmap_global_lock)
#undef NMG_LOCK_DESTROY
#define	NMG_LOCK_DESTROY()	NM_MTX_DESTROY(netmap_global_lock)
#undef NMG_LOCK
#define	NMG_LOCK()		NM_MTX_LOCK(netmap_global_lock)
#undef NMG_UNLOCK
#define	NMG_UNLOCK()		NM_MTX_UNLOCK(netmap_global_lock)
#undef NMG_LOCK_ASSERT
#define	NMG_LOCK_ASSERT()	NM_MTX_ASSERT(netmap_global_lock)
#undef nm_prinf
#define nm_prinf(format, ...)					 	do {							 		struct timeval __xxts;				 		microtime(&__xxts);				 		nm_prinf_int("%03d.%06d [%4d] %-25s " format "\n", 		(int)__xxts.tv_sec % 1000, (int)__xxts.tv_usec,	 		__LINE__, __FUNCTION__, ##__VA_ARGS__);		 	} while (0)
#undef nm_prerr
#define nm_prerr(format, ...)					 	do {							 		struct timeval __xxts;				 		microtime(&__xxts);				 		nm_prerr_int("%03d.%06d [%4d] %-25s " format "\n", 		(int)__xxts.tv_sec % 1000, (int)__xxts.tv_usec,	 		__LINE__, __FUNCTION__, ##__VA_ARGS__);		 	} while (0)
#undef nm_prdis
#define nm_prdis(format, ...)
#undef nm_prlim
#define nm_prlim(lps, format, ...)				 	do {							 		static int t0, __cnt;				 		if (t0 != time_second) {			 			t0 = time_second;			 			__cnt = 0;				 		}						 		if (__cnt++ < lps)				 			nm_prinf(format, ##__VA_ARGS__);	 	} while (0)
#undef for_rx_tx
#define for_rx_tx(t)	for ((t) = 0; (t) < NR_TXRX; (t)++)
#undef NKR_PENDINTR
#define NKR_PENDINTR	0x1		
#undef NKR_EXCLUSIVE
#define NKR_EXCLUSIVE	0x2		
#undef NKR_FORWARD
#define NKR_FORWARD	0x4		/* (host ring only) there are
#undef NKR_NEEDRING
#define NKR_NEEDRING	0x8		/* ring needed even if users==0
#undef NKR_NOINTR
#define NKR_NOINTR      0x10            
#undef NKR_FAKERING
#define NKR_FAKERING	0x20		
#undef NKR_NETMAP_OFF
#define NKR_NETMAP_OFF	0x0
#undef NKR_NETMAP_ON
#define NKR_NETMAP_ON	0x1
#undef NR_NOSLOT
#define NR_NOSLOT	((uint32_t)~0)	/* used in nkr_*lease* */
#undef plut_entry
#define plut_entry lut_entry
#undef NM_OS_MAGIC
#define NM_OS_MAGIC uint32_t
#undef NAF_SKIP_INTR
#define NAF_SKIP_INTR	1	/* use the regular interrupt handler.
#undef NAF_SW_ONLY
#define NAF_SW_ONLY	2	
#undef NAF_BDG_MAYSLEEP
#define NAF_BDG_MAYSLEEP 4	/* the bridge is allowed to sleep when
#undef NAF_MEM_OWNER
#define NAF_MEM_OWNER	8	/* the adapter uses its own memory area
#undef NAF_NATIVE
#define NAF_NATIVE      16      /* the adapter is native.
#undef NAF_NETMAP_ON
#define	NAF_NETMAP_ON	32	/* netmap is active (either native or
#undef NAF_HOST_RINGS
#define NAF_HOST_RINGS  64	
#undef NAF_FORCE_NATIVE
#define NAF_FORCE_NATIVE 128	
#undef NAF_MOREFRAG
#define NAF_MOREFRAG	512	
#undef NAF_OFFSETS
#define NAF_OFFSETS	1024	
#undef NAF_HOST_ALL
#define NAF_HOST_ALL	2048	
#undef NAF_ZOMBIE
#define NAF_ZOMBIE	(1U<<30) 
#undef NAF_BUSY
#define	NAF_BUSY	(1U<<31) /* the adapter is used internally and
#undef NAF_FORCE_READ
#define NAF_FORCE_READ      1
#undef NAF_FORCE_RECLAIM
#define NAF_FORCE_RECLAIM   2
#undef NAF_CAN_FORWARD_DOWN
#define NAF_CAN_FORWARD_DOWN 4
#undef NETMAP_OWNED_BY_KERN
#define NETMAP_OWNED_BY_KERN(na)	((na)->na_flags & NAF_BUSY)
#undef NETMAP_OWNED_BY_ANY
#define NETMAP_OWNED_BY_ANY(na)  	(NETMAP_OWNED_BY_KERN(na) || ((na)->active_fds > 0))
#undef netmap_vi_create
#define netmap_vi_create(hdr, a) (EOPNOTSUPP)
#undef NM_PIPE_ROLE_MASTER
#define NM_PIPE_ROLE_MASTER	0x1
#undef NM_PIPE_ROLE_SLAVE
#define NM_PIPE_ROLE_SLAVE	0x2
#undef nm_kr_txspace
#define nm_kr_txspace(_k) nm_kr_rxspace(_k)
#undef nm_kr_rxempty
#define nm_kr_rxempty(_k)	nm_kr_txempty(_k)
#undef NM_KR_BUSY
#define NM_KR_BUSY	1	
#undef NM_KR_STOPPED
#define NM_KR_STOPPED	2	
#undef NM_KR_LOCKED
#define NM_KR_LOCKED	3	
#undef netmap_tx_irq
#define netmap_tx_irq(_n, _q) netmap_rx_irq(_n, _q, NULL)
#undef NM_CHECK_ADDR_LEN_OFF
#define NM_CHECK_ADDR_LEN_OFF(na_, l_, o_) do {				 	if ((l_) + (o_) < (l_) || 					 	    (l_) + (o_) > NETMAP_BUF_SIZE(na_)) {			 		(l_) = NETMAP_BUF_SIZE(na_) - (o_);			 	} } while (0)
#undef netmap_bdg_learning
#define netmap_bdg_learning(_1, _2, _3, _4)	0
#undef netmap_get_vale_na
#define	netmap_get_vale_na(_1, _2, _3, _4)	0
#undef netmap_bdg_create
#define netmap_bdg_create(_1, _2)	NULL
#undef netmap_bdg_destroy
#define netmap_bdg_destroy(_1, _2)	0
#undef vale_max_bridges
#define vale_max_bridges		1
#undef netmap_pipe_alloc
#define netmap_pipe_alloc(_1, _2) 	0
#undef netmap_pipe_dealloc
#define netmap_pipe_dealloc(_1)
#undef netmap_get_pipe_na
#define netmap_get_pipe_na(hdr, _2, _3, _4)	 	((strchr(hdr->nr_name, '{') != NULL || strchr(hdr->nr_name, '}') != NULL) ? EOPNOTSUPP : 0)
#undef netmap_get_monitor_na
#define netmap_get_monitor_na(hdr, _2, _3, _4)  	(((struct nmreq_register *)(uintptr_t)hdr->nr_body)->nr_flags & (NR_MONITOR_TX | NR_MONITOR_RX) ? EOPNOTSUPP : 0)
#undef netmap_get_null_na
#define netmap_get_null_na(hdr, _2, _3, _4)  	(((struct nmreq_register *)(uintptr_t)hdr->nr_body)->nr_flags & (NR_MONITOR_TX | NR_MONITOR_RX) ? EOPNOTSUPP : 0)
#undef netmap_bns_get
#define netmap_bns_get()
#undef netmap_bns_put
#define netmap_bns_put(_1)
#undef netmap_bns_getbridges
#define netmap_bns_getbridges(b, n)  	do { *b = nm_bridges; *n = vale_max_bridges; } while (0)
#undef netmap_adapter_get
#define netmap_adapter_get(na) 				 	do {						 		struct netmap_adapter *__na = na;	 		__netmap_adapter_get(__na);		 		nm_prinf("getting %p:%s -> %d", __na, (__na)->name, (__na)->na_refcount);	 	} while (0)
#undef netmap_adapter_put
#define netmap_adapter_put(na)				 	({						 		struct netmap_adapter *__na = na;	 		if (__na == NULL)			 			nm_prinf("putting NULL");	 		else					 			nm_prinf("putting %p:%s -> %d", __na, (__na)->name, (__na)->na_refcount - 1);	 		__netmap_adapter_put(__na);	 	})
#undef NETMAP_BUF_BASE
#define NETMAP_BUF_BASE(_na)	((_na)->na_lut.lut[0].vaddr)
#undef NETMAP_BUF_SIZE
#define NETMAP_BUF_SIZE(_na)	((_na)->na_lut.objsize)
#undef netmap_debug
#define netmap_debug (0)
#undef NA
#define	NA(_ifp)	(if_getnetmapadapter(_ifp))
#undef NETMAP_MAGIC
#define	NETMAP_MAGIC	0x52697a7a
#undef NM_NA_VALID
#define NM_NA_VALID(ifp)	(NA(ifp) &&		 	((uint32_t)(uintptr_t)NA(ifp) ^ NA(ifp)->magic) == NETMAP_MAGIC )
#undef NM_ATTACH_NA
#define	NM_ATTACH_NA(ifp, na) do {					 	if_setnetmapadapter(ifp, na);					 	if (NA(ifp))							 		NA(ifp)->magic = 					 			((uint32_t)(uintptr_t)NA(ifp)) ^ NETMAP_MAGIC;	 } while(0)
#undef NM_RESTORE_NA
#define NM_RESTORE_NA(ifp, na) 	if_setnetmapadapter(ifp, na);
#undef NM_DETACH_NA
#define NM_DETACH_NA(ifp)	do { if_setnetmapadapter(ifp, NULL); } while (0)
#undef NM_NA_CLASH
#define NM_NA_CLASH(ifp)	(NA(ifp) && !NM_NA_VALID(ifp))
#undef NM_IS_NATIVE
#define NM_IS_NATIVE(ifp)	(NM_NA_VALID(ifp) && NA(ifp)->nm_dtor == netmap_hw_dtor)
#undef netmap_sync_map
#define netmap_sync_map(na, tag, map, sz, t)
#undef netmap_sync_map_cpu
#define netmap_sync_map_cpu(na, tag, map, sz, t)
#undef netmap_sync_map_dev
#define netmap_sync_map_dev(na, tag, map, sz, t)
#undef NM_BUF_ALIGN
#define NM_BUF_ALIGN	64
#undef NM_SYNC_KLOOP_RUNNING
#define NM_SYNC_KLOOP_RUNNING	(1 << 0)
#undef NM_SYNC_KLOOP_STOPPING
#define NM_SYNC_KLOOP_STOPPING	(1 << 1)
#undef generic_rate
#define generic_rate(txp, txs, txi, rxp, rxs, rxi)
#undef generic_netmap_attach
#define generic_netmap_attach(ifp)	(EOPNOTSUPP)
#undef na_is_generic
#define na_is_generic(na)		(0)
#undef VIRTIO_NET_HDR_F_NEEDS_CSUM
#define VIRTIO_NET_HDR_F_NEEDS_CSUM     1	
#undef VIRTIO_NET_HDR_F_DATA_VALID
#define VIRTIO_NET_HDR_F_DATA_VALID    2	
#undef VIRTIO_NET_HDR_GSO_NONE
#define VIRTIO_NET_HDR_GSO_NONE         0       
#undef VIRTIO_NET_HDR_GSO_TCPV4
#define VIRTIO_NET_HDR_GSO_TCPV4        1       
#undef VIRTIO_NET_HDR_GSO_UDP
#define VIRTIO_NET_HDR_GSO_UDP          3       
#undef VIRTIO_NET_HDR_GSO_TCPV6
#define VIRTIO_NET_HDR_GSO_TCPV6        4       
#undef VIRTIO_NET_HDR_GSO_ECN
#define VIRTIO_NET_HDR_GSO_ECN          0x80    
#undef WORST_CASE_GSO_HEADER
#define WORST_CASE_GSO_HEADER	(14+40+60)  
#undef rawsum_t
#define rawsum_t uint32_t
#undef SET_MBUF_DESTRUCTOR
#define SET_MBUF_DESTRUCTOR(m, fn, na)	do {		 	(m)->m_ext.ext_free = (fn != NULL) ?		 	    (void *)fn : (void *)nm_generic_mbuf_dtor;	 	(m)->m_ext.ext_arg1 = na;			 } while (0)
#undef ETH_HLEN
#define ETH_HLEN 6
#undef ETH_FCS_LEN
#define ETH_FCS_LEN 4
#undef VLAN_HLEN
#define VLAN_HLEN 4
struct netmap_adapter *netmap_getna(if_t ifp);
struct nm_selinfo { 	 	struct selinfo si; 	 	struct taskqueue *ntfytq; 	struct task ntfytask; 	struct mtx m; 	char mtxname[32]; 	int kqueue_users; };
struct hrtimer {      };
typedef struct hrtimer{ 	KTIMER timer; 	BOOLEAN active; 	KDPC deferred_proc; };
struct netmap_adapter;
struct nm_bdg_fwd;
struct nm_bridge;
struct netmap_priv_d;
struct nm_bdg_args;
enum txrx { NR_RX = 0, NR_TX = 1, NR_TXRX };
struct netmap_zmon_list { 	struct netmap_kring *next; 	struct netmap_kring *prev; };
struct netmap_kring { 	struct netmap_ring	*ring;  	uint32_t	nr_hwcur;   	uint32_t	nr_hwtail;  	     	uint32_t	rhead; 	uint32_t	rcur; 	uint32_t	rtail;  	uint32_t	nr_kflags;	 #define NKR_PENDINTR	0x1		 #define NKR_EXCLUSIVE	0x2		 #define NKR_FORWARD	0x4		   #define NKR_NEEDRING	0x8		    #define NKR_NOINTR      0x10             #define NKR_FAKERING	0x20		  	uint32_t	nr_mode; 	uint32_t	nr_pending_mode; #define NKR_NETMAP_OFF	0x0 #define NKR_NETMAP_ON	0x1  	uint32_t	nkr_num_slots;  	            	int32_t		nkr_hwofs; 	int32_t		nkr_to_refill;  	    	uint64_t	last_reclaim;   	NM_SELINFO_T	si;		 	NM_LOCK_T	q_lock;		 	NM_ATOMIC_T	nr_busy;	  	 	struct netmap_adapter *na;  	    	struct netmap_adapter *notify_na;  	 	struct nm_bdg_fwd *nkr_ft; 	uint32_t	*nkr_leases; #define NR_NOSLOT	((uint32_t)~0)	 	uint32_t	nkr_hwlease; 	uint32_t	nkr_lease_idx;  	      	volatile int nkr_stopped;  	      	struct mbuf	**tx_pool; 	struct mbuf	*tx_event;	 	NM_LOCK_T	tx_event_lock;	 #ifdef __FreeBSD__ 	struct callout	tx_event_callout; #endif 	struct mbq	rx_queue;         	uint32_t	users;		  	uint32_t	ring_id;	 	enum txrx	tx;		 	char name[64];			  	           	int (*nm_sync)(struct netmap_kring *kring, int flags); 	int (*nm_notify)(struct netmap_kring *kring, int flags);  #ifdef WITH_PIPES 	struct netmap_kring *pipe;	   	uint32_t pipe_tail;		 #endif   	 	uint64_t offset_mask; 	   	uint64_t offset_max; 	     	uint64_t offset_gap;  	    	uint64_t hwbuf_len;  	      	uint64_t buf_align;  	 	int (*nm_bufcfg)(struct netmap_kring *kring, uint64_t target);  	int (*save_notify)(struct netmap_kring *kring, int flags);  #ifdef WITH_MONITOR 	 	struct netmap_kring **monitors; 	uint32_t max_monitors;  	uint32_t n_monitors;	 	uint32_t mon_pos[NR_TXRX];  	uint32_t mon_tail;    	 	struct netmap_zmon_list zmon_list[NR_TXRX];  	     	int (*mon_sync)(struct netmap_kring *kring, int flags); 	int (*mon_notify)(struct netmap_kring *kring, int flags);  #endif } #ifdef _WIN32 __declspec(align(64));
struct lut_entry;
struct netmap_lut { 	struct lut_entry *lut; 	struct plut_entry *plut; 	uint32_t objtotal;	 	uint32_t objsize;	 };
struct netmap_vp_adapter;
struct nm_bridge;
struct nm_config_info { 	unsigned num_tx_rings; 	unsigned num_rx_rings; 	unsigned num_tx_descs; 	unsigned num_rx_descs; 	unsigned rx_buf_maxsize; };
struct netmap_adapter { 	       	NM_OS_MAGIC magic; 	uint32_t na_flags;	 #define NAF_SKIP_INTR	1	   #define NAF_SW_ONLY	2	 #define NAF_BDG_MAYSLEEP 4	    #define NAF_MEM_OWNER	8	   #define NAF_NATIVE      16           #define	NAF_NETMAP_ON	32	    #define NAF_HOST_RINGS  64	 #define NAF_FORCE_NATIVE 128	  #define NAF_MOREFRAG	512	 #define NAF_OFFSETS	1024	 #define NAF_HOST_ALL	2048	 #define NAF_ZOMBIE	(1U<<30)  #define	NAF_BUSY	(1U<<31)    	int active_fds;     	u_int num_rx_rings;  	u_int num_tx_rings;  	u_int num_host_rx_rings;  	u_int num_host_tx_rings;   	u_int num_tx_desc;   	u_int num_rx_desc;  	    	struct netmap_kring **tx_rings;  	struct netmap_kring **rx_rings;   	void *tailroom;		        				          	NM_SELINFO_T si[NR_TXRX];	  	 	int si_users[NR_TXRX];  	void *pdev;   	   	int     (*if_transmit)(if_t, struct mbuf *);  	 	void     (*if_input)(if_t, struct mbuf *);  	  	if_t ifp;   	 	                                                     	void (*nm_dtor)(struct netmap_adapter *);  	int (*nm_register)(struct netmap_adapter *, int onoff); 	void (*nm_intr)(struct netmap_adapter *, int onoff);  	int (*nm_txsync)(struct netmap_kring *kring, int flags); 	int (*nm_rxsync)(struct netmap_kring *kring, int flags); 	int (*nm_notify)(struct netmap_kring *kring, int flags); 	int (*nm_bufcfg)(struct netmap_kring *kring, uint64_t target); #define NAF_FORCE_READ      1 #define NAF_FORCE_RECLAIM   2 #define NAF_CAN_FORWARD_DOWN 4 	 	int (*nm_config)(struct netmap_adapter *, struct nm_config_info *info); 	int (*nm_krings_create)(struct netmap_adapter *); 	void (*nm_krings_delete)(struct netmap_adapter *); 	               	int (*nm_bdg_attach)(const char *bdg_name, struct netmap_adapter *, 			struct nm_bridge *); 	int (*nm_bdg_ctl)(struct nmreq_header *, struct netmap_adapter *);  	 	struct netmap_vp_adapter *na_vp; 	  	struct netmap_vp_adapter *na_hostvp;  	   	int na_refcount;  	     	struct netmap_mem_d *nm_mem; 	struct netmap_mem_d *nm_mem_prev; 	struct netmap_lut na_lut;  	    	void *na_private;  	 	struct netmap_pipe_adapter **na_pipes; 	int na_next_pipe;	 	int na_max_pipes;	  	 	u_int virt_hdr_len;  	    	unsigned rx_buf_maxsize;  	char name[NETMAP_REQ_IFNAMSIZ];   #ifdef WITH_MONITOR 	unsigned long	monitor_id;	 #endif };
struct netmap_vp_adapter {	 	struct netmap_adapter up;  	      	int bdg_port; 	struct nm_bridge *na_bdg; 	int retry; 	int autodelete;   	 	u_int mfs; 	 	uint64_t last_smac; };
struct netmap_hw_adapter {	 	struct netmap_adapter up;  #ifdef linux 	struct net_device_ops nm_ndo; 	struct ethtool_ops    nm_eto; #endif 	const struct ethtool_ops*   save_ethtool;  	int (*nm_hw_register)(struct netmap_adapter *, int onoff); };
struct nm_generic_mit { 	struct hrtimer mit_timer; 	int mit_pending; 	int mit_ring_idx;   	struct netmap_adapter *mit_na;   };
struct netmap_generic_adapter {	 	struct netmap_hw_adapter up;  	 	struct netmap_adapter *prev;  	   	struct nm_generic_mit *mit; #ifdef linux         netdev_tx_t (*save_start_xmit)(struct mbuf *, if_t); #endif 	  	int rxsg;  	  	int txqdisc; };
struct nm_bdg_polling_state;
struct netmap_bwrap_adapter { 	struct netmap_vp_adapter up; 	struct netmap_vp_adapter host;   	struct netmap_adapter *hwna;	  	       	struct netmap_priv_d *na_kpriv; 	struct nm_bdg_polling_state *na_polling_state; 	   	struct netmap_vp_adapter *saved_na_vp; 	int (*nm_intr_notify)(struct netmap_kring *kring, int flags); };
struct netmap_pipe_adapter { 	 	struct netmap_adapter up;  #define NM_PIPE_ROLE_MASTER	0x1 #define NM_PIPE_ROLE_SLAVE	0x2 	int role;	  	struct netmap_adapter *parent;  	struct netmap_pipe_adapter *peer;  	int peer_ref;		 	if_t parent_ifp;	  	u_int parent_slot;  };
struct netmap_null_adapter { 	struct netmap_adapter up; };
struct netmap_slot *netmap_reset(struct netmap_adapter *na, 	enum txrx tx, u_int n, u_int new_cur);
enum { 	  	NM_IRQ_PASS = 0, 	   	NM_IRQ_COMPLETED = -1, 	    	NM_IRQ_RESCHED = -2, };
struct net *netmap_bns_get(void);
enum {                                   	NM_DEBUG_ON = 1,		 	NM_DEBUG_HOST = 0x2,             	NM_DEBUG_RXSYNC = 0x10,          	NM_DEBUG_TXSYNC = 0x20, 	NM_DEBUG_RXINTR = 0x100,         	NM_DEBUG_TXINTR = 0x200, 	NM_DEBUG_NIC_RXSYNC = 0x1000,    	NM_DEBUG_NIC_TXSYNC = 0x2000, 	NM_DEBUG_MEM = 0x4000,		 	NM_DEBUG_VALE = 0x8000,		 	NM_DEBUG_BDG = NM_DEBUG_VALE, };
struct e1000_buffer *buffer_info =  &tx_ring->buffer_info[l];
struct lut_entry { 	void *vaddr;		 	vm_paddr_t paddr;	 };
struct lut_entry { 	void *vaddr;		 };
struct plut_entry { 	vm_paddr_t paddr;	 };
struct netmap_obj_pool;
struct netmap_priv_d { 	struct netmap_if * volatile np_nifp;	  	struct netmap_adapter	*np_na; 	if_t		np_ifp; 	uint32_t	np_flags;	 	u_int		np_qfirst[NR_TXRX], 			np_qlast[NR_TXRX];  	uint16_t	np_txpoll; 	uint16_t        np_kloop_state;	 #define NM_SYNC_KLOOP_RUNNING	(1 << 0) #define NM_SYNC_KLOOP_STOPPING	(1 << 1) 	int             np_sync_flags;   	int		np_refs;	  	    	NM_SELINFO_T *np_si[NR_TXRX];  	          	struct nm_csb_atok	*np_csb_atok_base; 	  	struct nm_csb_ktoa	*np_csb_ktoa_base;  #ifdef linux 	struct file	*np_filp;   #endif  };
struct netmap_priv_d *netmap_priv_new(void);
struct netmap_monitor_adapter { 	struct netmap_adapter up;  	struct netmap_priv_d priv; 	uint32_t flags; };
struct nm_os_gen_arg { 	if_t ifp; 	void *m;	 	void *head, *tail;  	void *addr;	 	u_int len;	 	u_int ring_nr;	 	u_int qevent;    };
struct nm_bdg_fwd {	 	void *ft_buf;		 	uint8_t ft_frags;	 	uint16_t ft_offset;	 	uint16_t ft_flags;	 	uint16_t ft_len;	 	uint16_t ft_next;	 };
struct nm_vnet_hdr { #define VIRTIO_NET_HDR_F_NEEDS_CSUM     1	 #define VIRTIO_NET_HDR_F_DATA_VALID    2	     uint8_t flags; #define VIRTIO_NET_HDR_GSO_NONE         0        #define VIRTIO_NET_HDR_GSO_TCPV4        1        #define VIRTIO_NET_HDR_GSO_UDP          3        #define VIRTIO_NET_HDR_GSO_TCPV6        4        #define VIRTIO_NET_HDR_GSO_ECN          0x80         uint8_t gso_type;     uint16_t hdr_len;     uint16_t gso_size;     uint16_t csum_start;     uint16_t csum_offset; };
struct nm_iphdr { 	uint8_t		version_ihl; 	uint8_t		tos; 	uint16_t	tot_len; 	uint16_t	id; 	uint16_t	frag_off; 	uint8_t		ttl; 	uint8_t		protocol; 	uint16_t	check; 	uint32_t	saddr; 	uint32_t	daddr; 	 };
struct nm_tcphdr { 	uint16_t	source; 	uint16_t	dest; 	uint32_t	seq; 	uint32_t	ack_seq; 	uint8_t		doff;   	uint8_t		flags; 	uint16_t	window; 	uint16_t	check; 	uint16_t	urg_ptr; };
struct nm_udphdr { 	uint16_t	source; 	uint16_t	dest; 	uint16_t	len; 	uint16_t	check; };
struct nm_ipv6hdr { 	uint8_t		priority_version; 	uint8_t		flow_lbl[3];  	uint16_t	payload_len; 	uint8_t		nexthdr; 	uint8_t		hop_limit;  	uint8_t		saddr[16]; 	uint8_t		daddr[16]; };
struct nm_kctx;
typedef void (*nm_kctx_worker_fn_t)(void *data);
struct nm_kctx_cfg { 	long			type;		 	nm_kctx_worker_fn_t	worker_fn;	 	void			*worker_private; 	int			attach_user;	 };
struct nm_kctx *nm_os_kctx_create(struct nm_kctx_cfg *cfg, 					void *opaque);
struct ptnetmap_memdev;
struct netmap_pt_guest_adapter {           	struct netmap_hw_adapter hwup;                   struct netmap_hw_adapter dr;  	    	int backend_users;  };
struct nmreq_option * nmreq_getoption(struct nmreq_header *, uint16_t);
/*@ requires lim > 0; ensures \result == (i == 0 ? lim : i - 1); */
static inline uint32_t
nm_prev(uint32_t i, uint32_t lim)
{
	return unlikely (i == 0) ? lim : i - 1;
}
