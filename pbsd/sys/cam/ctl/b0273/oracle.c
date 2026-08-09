/*
 * oracle.c - reference implementation for PBSD batch b0273.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef LONG_BIT
#define LONG_BIT (8 * (int)sizeof(long))
#endif

#define le16toh(x) (x)
#define CTL_TIME_IO
#define CTL_NUM_PRIV 6
#define CTL_BE_NAME_LEN 32

#define QMD_STAILQ_CHECK_TAIL(head) do {} while (0)
#define QMD_SAVELINK(name, link) uintptr_t name = (uintptr_t)(link)
#define TRASHIT(x) do {(void)(x);} while (0)
#define QUEUE_TYPEOF(type) struct type

#define STAILQ_HEAD(name, type) struct name { struct type *stqh_first; struct type **stqh_last; }
#define STAILQ_ENTRY(type) struct { struct type *stqe_next; }
#define STAILQ_FIRST(head) ((head)->stqh_first)
#define STAILQ_NEXT(elm, field) ((elm)->field.stqe_next)
#define STAILQ_EMPTY(head) (STAILQ_FIRST(head) == NULL)
#define STAILQ_INIT(head) do { STAILQ_FIRST((head)) = NULL; (head)->stqh_last = &STAILQ_FIRST((head)); } while (0)
#define STAILQ_FOREACH(var, head, field) for ((var) = STAILQ_FIRST((head)); (var); (var) = STAILQ_NEXT((var), field))
#define STAILQ_INSERT_TAIL(head, elm, field) do { \
	__typeof__((head)->stqh_last) prevlast = (head)->stqh_last; \
	STAILQ_NEXT((elm), field) = NULL; \
	(head)->stqh_last = &STAILQ_NEXT((elm), field); \
	*prevlast = (elm); \
} while (0)
#define STAILQ_REMOVE(head, elm, type, field) do { \
	QMD_SAVELINK(_Oldnext, (elm)->field.stqe_next); \
	if (STAILQ_FIRST((head)) == (elm)) { \
		if ((STAILQ_FIRST((head)) = STAILQ_NEXT(STAILQ_FIRST((head)), field)) == NULL) \
			(head)->stqh_last = &STAILQ_FIRST((head)); \
	} else { \
		QUEUE_TYPEOF(type) *_Curelm = STAILQ_FIRST(head); \
		while (STAILQ_NEXT(_Curelm, field) != (elm)) \
			_Curelm = STAILQ_NEXT(_Curelm, field); \
		if ((STAILQ_NEXT(_Curelm, field) = STAILQ_NEXT(STAILQ_NEXT(_Curelm, field), field)) == NULL) \
			(head)->stqh_last = &STAILQ_NEXT((_Curelm), field); \
	} \
	TRASHIT(*_Oldnext); \
} while (0)

typedef unsigned int u_int;
typedef unsigned long u_long;
typedef char *caddr_t;
typedef int ctl_io_flags;

typedef enum {
	CTL_IO_NONE,
	CTL_IO_SCSI,
	CTL_IO_TASK,
	CTL_IO_NVME,
	CTL_IO_NVME_ADMIN,
} ctl_io_type;

typedef enum {
	CTL_SER_SEQ,
	CTL_SER_PASS,
	CTL_SER_EXTENTOPT,
	CTL_SER_EXTENT,
	CTL_SER_BLOCKOPT,
	CTL_SER_BLOCK,
} ctl_serialize_action;

typedef enum {
	CTL_SERIDX_TUR = 0,
	CTL_SERIDX_READ,
	CTL_SERIDX_WRITE,
	CTL_SERIDX_UNMAP,
	CTL_SERIDX_SYNC,
	CTL_SERIDX_MD_SNS,
	CTL_SERIDX_MD_SEL,
	CTL_SERIDX_RQ_SNS,
	CTL_SERIDX_INQ,
	CTL_SERIDX_RD_CAP,
	CTL_SERIDX_RES,
	CTL_SERIDX_LOG_SNS,
	CTL_SERIDX_FORMAT,
	CTL_SERIDX_START,
	CTL_SERIDX_COUNT,
} ctl_seridx_t;

#define CTL_FLAG_DATA_IN 0x00000001
#define CTL_FLAG_DATA_OUT 0x00000002
#define CTL_FLAG_DATA_NONE 0x00000003
#define CTL_CMD_FLAG_OK_ON_NO_LUN 0x0800

#define NVME_STATUS_P_SHIFT 0
#define NVME_STATUS_P_MASK 0x1
#define NVME_STATUS_SC_SHIFT 1
#define NVME_STATUS_SC_MASK 0xFF
#define NVME_STATUS_SCT_SHIFT 9
#define NVME_STATUS_SCT_MASK 0x7
#define NVME_STATUS_CRD_SHIFT 12
#define NVME_STATUS_CRD_MASK 0x3
#define NVME_STATUS_M_SHIFT 14
#define NVME_STATUS_M_MASK 0x1
#define NVME_STATUS_DNR_SHIFT 15
#define NVME_STATUS_DNR_MASK 0x1
#define NVMEV(name, x) (((x) >> name##_SHIFT) & name##_MASK)
#define NVME_STATUS_GET_SC(st) NVMEV(NVME_STATUS_SC, st)
#define NVME_STATUS_GET_SCT(st) NVMEV(NVME_STATUS_SCT, st)
#define NVME_STATUS_GET_M(st) NVMEV(NVME_STATUS_M, st)
#define NVME_STATUS_GET_DNR(st) NVMEV(NVME_STATUS_DNR, st)

enum nvme_status_code_type {
	NVME_SCT_GENERIC = 0x0,
	NVME_SCT_COMMAND_SPECIFIC = 0x1,
	NVME_SCT_MEDIA_ERROR = 0x2,
	NVME_SCT_PATH_RELATED = 0x3,
	NVME_SCT_VENDOR_SPECIFIC = 0x7,
};

enum nvme_generic_command_status_code {
	NVME_SC_SUCCESS = 0x00,
	NVME_SC_INVALID_OPCODE = 0x01,
	NVME_SC_INVALID_FIELD = 0x02,
	NVME_SC_COMMAND_ID_CONFLICT = 0x03,
	NVME_SC_DATA_TRANSFER_ERROR = 0x04,
	NVME_SC_ABORTED_POWER_LOSS = 0x05,
	NVME_SC_INTERNAL_DEVICE_ERROR = 0x06,
	NVME_SC_ABORTED_BY_REQUEST = 0x07,
	NVME_SC_ABORTED_SQ_DELETION = 0x08,
	NVME_SC_ABORTED_FAILED_FUSED = 0x09,
	NVME_SC_ABORTED_MISSING_FUSED = 0x0a,
	NVME_SC_INVALID_NAMESPACE_OR_FORMAT = 0x0b,
	NVME_SC_COMMAND_SEQUENCE_ERROR = 0x0c,
	NVME_SC_INVALID_SGL_SEGMENT_DESCR = 0x0d,
	NVME_SC_INVALID_NUMBER_OF_SGL_DESCR = 0x0e,
	NVME_SC_DATA_SGL_LENGTH_INVALID = 0x0f,
	NVME_SC_METADATA_SGL_LENGTH_INVALID = 0x10,
	NVME_SC_SGL_DESCRIPTOR_TYPE_INVALID = 0x11,
	NVME_SC_INVALID_USE_OF_CMB = 0x12,
	NVME_SC_PRP_OFFET_INVALID = 0x13,
	NVME_SC_ATOMIC_WRITE_UNIT_EXCEEDED = 0x14,
	NVME_SC_OPERATION_DENIED = 0x15,
	NVME_SC_SGL_OFFSET_INVALID = 0x16,
	NVME_SC_HOST_ID_INCONSISTENT_FORMAT = 0x18,
	NVME_SC_KEEP_ALIVE_TIMEOUT_EXPIRED = 0x19,
	NVME_SC_KEEP_ALIVE_TIMEOUT_INVALID = 0x1a,
	NVME_SC_ABORTED_DUE_TO_PREEMPT = 0x1b,
	NVME_SC_SANITIZE_FAILED = 0x1c,
	NVME_SC_SANITIZE_IN_PROGRESS = 0x1d,
	NVME_SC_SGL_DATA_BLOCK_GRAN_INVALID = 0x1e,
	NVME_SC_NOT_SUPPORTED_IN_CMB = 0x1f,
	NVME_SC_NAMESPACE_IS_WRITE_PROTECTED = 0x20,
	NVME_SC_COMMAND_INTERRUPTED = 0x21,
	NVME_SC_TRANSIENT_TRANSPORT_ERROR = 0x22,
	NVME_SC_LBA_OUT_OF_RANGE = 0x80,
	NVME_SC_CAPACITY_EXCEEDED = 0x81,
	NVME_SC_NAMESPACE_NOT_READY = 0x82,
	NVME_SC_RESERVATION_CONFLICT = 0x83,
	NVME_SC_FORMAT_IN_PROGRESS = 0x84,
};

enum nvme_command_specific_status_code {
	NVME_SC_COMPLETION_QUEUE_INVALID = 0x00,
	NVME_SC_INVALID_QUEUE_IDENTIFIER = 0x01,
	NVME_SC_MAXIMUM_QUEUE_SIZE_EXCEEDED = 0x02,
	NVME_SC_ABORT_COMMAND_LIMIT_EXCEEDED = 0x03,
	NVME_SC_ASYNC_EVENT_REQUEST_LIMIT_EXCEEDED = 0x05,
	NVME_SC_INVALID_FIRMWARE_SLOT = 0x06,
	NVME_SC_INVALID_FIRMWARE_IMAGE = 0x07,
	NVME_SC_INVALID_INTERRUPT_VECTOR = 0x08,
	NVME_SC_INVALID_LOG_PAGE = 0x09,
	NVME_SC_INVALID_FORMAT = 0x0a,
	NVME_SC_FIRMWARE_REQUIRES_RESET = 0x0b,
	NVME_SC_INVALID_QUEUE_DELETION = 0x0c,
	NVME_SC_FEATURE_NOT_SAVEABLE = 0x0d,
	NVME_SC_FEATURE_NOT_CHANGEABLE = 0x0e,
	NVME_SC_FEATURE_NOT_NS_SPECIFIC = 0x0f,
	NVME_SC_FW_ACT_REQUIRES_NVMS_RESET = 0x10,
	NVME_SC_FW_ACT_REQUIRES_RESET = 0x11,
	NVME_SC_FW_ACT_REQUIRES_TIME = 0x12,
	NVME_SC_FW_ACT_PROHIBITED = 0x13,
	NVME_SC_OVERLAPPING_RANGE = 0x14,
	NVME_SC_NS_INSUFFICIENT_CAPACITY = 0x15,
	NVME_SC_NS_ID_UNAVAILABLE = 0x16,
	NVME_SC_NS_ALREADY_ATTACHED = 0x18,
	NVME_SC_NS_IS_PRIVATE = 0x19,
	NVME_SC_NS_NOT_ATTACHED = 0x1a,
	NVME_SC_THIN_PROV_NOT_SUPPORTED = 0x1b,
	NVME_SC_CTRLR_LIST_INVALID = 0x1c,
	NVME_SC_SELF_TEST_IN_PROGRESS = 0x1d,
	NVME_SC_BOOT_PART_WRITE_PROHIB = 0x1e,
	NVME_SC_INVALID_CTRLR_ID = 0x1f,
	NVME_SC_INVALID_SEC_CTRLR_STATE = 0x20,
	NVME_SC_INVALID_NUM_OF_CTRLR_RESRC = 0x21,
	NVME_SC_INVALID_RESOURCE_ID = 0x22,
	NVME_SC_SANITIZE_PROHIBITED_WPMRE = 0x23,
	NVME_SC_ANA_GROUP_ID_INVALID = 0x24,
	NVME_SC_ANA_ATTACH_FAILED = 0x25,
	NVME_SC_CONFLICTING_ATTRIBUTES = 0x80,
	NVME_SC_INVALID_PROTECTION_INFO = 0x81,
	NVME_SC_ATTEMPTED_WRITE_TO_RO_PAGE = 0x82,
};

enum nvme_media_error_status_code {
	NVME_SC_WRITE_FAULTS = 0x80,
	NVME_SC_UNRECOVERED_READ_ERROR = 0x81,
	NVME_SC_GUARD_CHECK_ERROR = 0x82,
	NVME_SC_APPLICATION_TAG_CHECK_ERROR = 0x83,
	NVME_SC_REFERENCE_TAG_CHECK_ERROR = 0x84,
	NVME_SC_COMPARE_FAILURE = 0x85,
	NVME_SC_ACCESS_DENIED = 0x86,
	NVME_SC_DEALLOCATED_OR_UNWRITTEN = 0x87,
};

enum nvme_path_related_status_code {
	NVME_SC_INTERNAL_PATH_ERROR = 0x00,
	NVME_SC_ASYMMETRIC_ACCESS_PERSISTENT_LOSS = 0x01,
	NVME_SC_ASYMMETRIC_ACCESS_INACCESSIBLE = 0x02,
	NVME_SC_ASYMMETRIC_ACCESS_TRANSITION = 0x03,
	NVME_SC_CONTROLLER_PATHING_ERROR = 0x60,
	NVME_SC_HOST_PATHING_ERROR = 0x70,
	NVME_SC_COMMAND_ABORTED_BY_HOST = 0x71,
};

enum nvme_admin_opcode {
	NVME_OPC_DELETE_IO_SQ = 0x00,
	NVME_OPC_CREATE_IO_SQ = 0x01,
	NVME_OPC_GET_LOG_PAGE = 0x02,
	NVME_OPC_DELETE_IO_CQ = 0x04,
	NVME_OPC_CREATE_IO_CQ = 0x05,
	NVME_OPC_IDENTIFY = 0x06,
	NVME_OPC_ABORT = 0x08,
	NVME_OPC_SET_FEATURES = 0x09,
	NVME_OPC_GET_FEATURES = 0x0a,
	NVME_OPC_ASYNC_EVENT_REQUEST = 0x0c,
	NVME_OPC_NAMESPACE_MANAGEMENT = 0x0d,
	NVME_OPC_FIRMWARE_ACTIVATE = 0x10,
	NVME_OPC_FIRMWARE_IMAGE_DOWNLOAD = 0x11,
	NVME_OPC_DEVICE_SELF_TEST = 0x14,
	NVME_OPC_NAMESPACE_ATTACHMENT = 0x15,
	NVME_OPC_KEEP_ALIVE = 0x18,
	NVME_OPC_DIRECTIVE_SEND = 0x19,
	NVME_OPC_DIRECTIVE_RECEIVE = 0x1a,
	NVME_OPC_VIRTUALIZATION_MANAGEMENT = 0x1c,
	NVME_OPC_NVME_MI_SEND = 0x1d,
	NVME_OPC_NVME_MI_RECEIVE = 0x1e,
	NVME_OPC_CAPACITY_MANAGEMENT = 0x20,
	NVME_OPC_LOCKDOWN = 0x24,
	NVME_OPC_DOORBELL_BUFFER_CONFIG = 0x7c,
	NVME_OPC_FABRICS_COMMANDS = 0x7f,
	NVME_OPC_FORMAT_NVM = 0x80,
	NVME_OPC_SECURITY_SEND = 0x81,
	NVME_OPC_SECURITY_RECEIVE = 0x82,
	NVME_OPC_SANITIZE = 0x84,
	NVME_OPC_GET_LBA_STATUS = 0x86,
};

enum nvme_nvm_opcode {
	NVME_OPC_FLUSH = 0x00,
	NVME_OPC_WRITE = 0x01,
	NVME_OPC_READ = 0x02,
	NVME_OPC_WRITE_UNCORRECTABLE = 0x04,
	NVME_OPC_COMPARE = 0x05,
	NVME_OPC_WRITE_ZEROES = 0x08,
	NVME_OPC_DATASET_MANAGEMENT = 0x09,
	NVME_OPC_VERIFY = 0x0c,
	NVME_OPC_RESERVATION_REGISTER = 0x0d,
	NVME_OPC_RESERVATION_REPORT = 0x0e,
	NVME_OPC_RESERVATION_ACQUIRE = 0x11,
	NVME_OPC_RESERVATION_RELEASE = 0x15,
	NVME_OPC_COPY = 0x19,
};

struct sbuf {
	char *s_buf;
	void *s_drain_func;
	void *s_drain_arg;
	int s_error;
	long s_size;
	long s_len;
	int s_flags;
	long s_sect_len;
	long s_rec_off;
};

struct nvme_command {
	uint8_t opc, fuse;
	uint16_t cid;
	uint32_t nsid;
	uint32_t rsvd2, rsvd3;
	uint64_t mptr;
	uint64_t prp1, prp2;
	uint32_t cdw10, cdw11, cdw12, cdw13, cdw14, cdw15;
} __attribute__((aligned(8)));

struct nvme_completion {
	uint32_t cdw0, rsvd1;
	uint16_t sqhd, sqid;
	uint16_t cid, status;
} __attribute__((aligned(8)));

typedef enum { CTL_MSG_SERIALIZE } ctl_msg_type;
struct ctl_nexus { uint32_t initid, targ_port, targ_lun, targ_mapped_lun; };
union ctl_io;
union ctl_priv { uint8_t bytes[16]; uint64_t integer; uint64_t integers[2]; void *ptr; void *ptrs[2]; };
struct bintime { time_t sec; uint64_t frac; };
struct ctl_io_hdr {
	uint32_t version;
	ctl_io_type io_type;
	ctl_msg_type msg_type;
	struct ctl_nexus nexus;
	uint32_t iid_indx, flags, status, port_status, timeout, retries;
	time_t start_time;
	struct bintime start_bt, dma_start_bt, dma_bt;
	uint32_t num_dmas;
	union ctl_io *remote_io, *blocker;
	void *pool;
	union ctl_priv ctl_private[CTL_NUM_PRIV];
	struct { struct ctl_io_hdr *tqh_first, **tqh_last; } blocked_queue;
	struct { struct ctl_io_hdr *stqe_next; } links;
	struct { struct ctl_io_hdr *le_next, **le_prev; } ooa_links;
	struct { struct ctl_io_hdr *tqe_next, **tqe_prev; } blocked_links;
};
typedef void (*ctl_ref)(void *, int);
typedef int (*ctl_be_move_done_t)(union ctl_io *, bool);
typedef int (*ctl_io_cont)(union ctl_io *);
struct ctl_nvmeio {
	struct ctl_io_hdr io_hdr;
	uint32_t ext_sg_entries;
	uint8_t *ext_data_ptr;
	uint32_t ext_data_len, ext_data_filled;
	uint32_t kern_sg_entries;
	uint8_t *kern_data_ptr;
	uint32_t kern_data_len, kern_total_len, kern_data_resid, kern_rel_offset;
	struct nvme_command cmd;
	struct nvme_completion cpl;
	bool success_sent;
	ctl_be_move_done_t be_move_done;
	ctl_io_cont io_cont;
	ctl_ref kern_data_ref;
	void *kern_data_arg;
};

struct ctl_nvme_cmd_entry {
	int (*execute)(struct ctl_nvmeio *);
	ctl_io_flags flags;
};

typedef int (*be_init_t)(void);
typedef int (*be_shutdown_t)(void);
struct ctl_backend_driver {
	char name[CTL_BE_NAME_LEN];
	int flags;
	be_init_t init;
	be_shutdown_t shutdown;
	void *data_submit;
	void *config_read;
	void *config_write;
	void *ioctl;
	void *lun_info;
	void *lun_attr;
	STAILQ_ENTRY(ctl_backend_driver) links;
};

struct mtx { int locked; };
struct ctl_softc {
	struct mtx ctl_lock;
	uint32_t num_backends;
	STAILQ_HEAD(, ctl_backend_driver) be_list;
};

static struct ctl_softc ref_ctl_softc_storage;
struct ctl_softc *control_softc = &ref_ctl_softc_storage;

static void mtx_lock(struct mtx *m) { (void)m; }
static void mtx_unlock(struct mtx *m) { (void)m; }

static int sbuf_printf(struct sbuf *sb, const char *fmt, ...)
{
	va_list ap;
	int n;
	va_start(ap, fmt);
	n = vsnprintf(sb->s_buf + sb->s_len, (size_t)(sb->s_size - sb->s_len), fmt, ap);
	va_end(ap);
	if (n < 0)
		return n;
	if (sb->s_len + n >= sb->s_size)
		n = (int)(sb->s_size - sb->s_len - 1);
	sb->s_len += n;
	return n;
}

int ctl_nvme_identify(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_flush(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_read_write(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_write_uncorrectable(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_compare(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_write_zeroes(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_dataset_management(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_verify(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }

#define OPC_ENTRY(x)		[NVME_OPC_ ## x] = #x

static const char *admin_opcode[256] = {
	OPC_ENTRY(DELETE_IO_SQ),
	OPC_ENTRY(CREATE_IO_SQ),
	OPC_ENTRY(GET_LOG_PAGE),
	OPC_ENTRY(DELETE_IO_CQ),
	OPC_ENTRY(CREATE_IO_CQ),
	OPC_ENTRY(IDENTIFY),
	OPC_ENTRY(ABORT),
	OPC_ENTRY(SET_FEATURES),
	OPC_ENTRY(GET_FEATURES),
	OPC_ENTRY(ASYNC_EVENT_REQUEST),
	OPC_ENTRY(NAMESPACE_MANAGEMENT),
	OPC_ENTRY(FIRMWARE_ACTIVATE),
	OPC_ENTRY(FIRMWARE_IMAGE_DOWNLOAD),
	OPC_ENTRY(DEVICE_SELF_TEST),
	OPC_ENTRY(NAMESPACE_ATTACHMENT),
	OPC_ENTRY(KEEP_ALIVE),
	OPC_ENTRY(DIRECTIVE_SEND),
	OPC_ENTRY(DIRECTIVE_RECEIVE),
	OPC_ENTRY(VIRTUALIZATION_MANAGEMENT),
	OPC_ENTRY(NVME_MI_SEND),
	OPC_ENTRY(NVME_MI_RECEIVE),
	OPC_ENTRY(CAPACITY_MANAGEMENT),
	OPC_ENTRY(LOCKDOWN),
	OPC_ENTRY(DOORBELL_BUFFER_CONFIG),
	OPC_ENTRY(FABRICS_COMMANDS),
	OPC_ENTRY(FORMAT_NVM),
	OPC_ENTRY(SECURITY_SEND),
	OPC_ENTRY(SECURITY_RECEIVE),
	OPC_ENTRY(SANITIZE),
	OPC_ENTRY(GET_LBA_STATUS),
};

static const char *nvm_opcode[256] = {
	OPC_ENTRY(FLUSH),
	OPC_ENTRY(WRITE),
	OPC_ENTRY(READ),
	OPC_ENTRY(WRITE_UNCORRECTABLE),
	OPC_ENTRY(COMPARE),
	OPC_ENTRY(WRITE_ZEROES),
	OPC_ENTRY(DATASET_MANAGEMENT),
	OPC_ENTRY(VERIFY),
	OPC_ENTRY(RESERVATION_REGISTER),
	OPC_ENTRY(RESERVATION_REPORT),
	OPC_ENTRY(RESERVATION_ACQUIRE),
	OPC_ENTRY(RESERVATION_RELEASE),
	OPC_ENTRY(COPY),
};

#define SC_ENTRY(x)		[NVME_SC_ ## x] = #x

static const char *generic_status[256] = {
	SC_ENTRY(SUCCESS),
	SC_ENTRY(INVALID_OPCODE),
	SC_ENTRY(INVALID_FIELD),
	SC_ENTRY(COMMAND_ID_CONFLICT),
	SC_ENTRY(DATA_TRANSFER_ERROR),
	SC_ENTRY(ABORTED_POWER_LOSS),
	SC_ENTRY(INTERNAL_DEVICE_ERROR),
	SC_ENTRY(ABORTED_BY_REQUEST),
	SC_ENTRY(ABORTED_SQ_DELETION),
	SC_ENTRY(ABORTED_FAILED_FUSED),
	SC_ENTRY(ABORTED_MISSING_FUSED),
	SC_ENTRY(INVALID_NAMESPACE_OR_FORMAT),
	SC_ENTRY(COMMAND_SEQUENCE_ERROR),
	SC_ENTRY(INVALID_SGL_SEGMENT_DESCR),
	SC_ENTRY(INVALID_NUMBER_OF_SGL_DESCR),
	SC_ENTRY(DATA_SGL_LENGTH_INVALID),
	SC_ENTRY(METADATA_SGL_LENGTH_INVALID),
	SC_ENTRY(SGL_DESCRIPTOR_TYPE_INVALID),
	SC_ENTRY(INVALID_USE_OF_CMB),
	SC_ENTRY(PRP_OFFET_INVALID),
	SC_ENTRY(ATOMIC_WRITE_UNIT_EXCEEDED),
	SC_ENTRY(OPERATION_DENIED),
	SC_ENTRY(SGL_OFFSET_INVALID),
	SC_ENTRY(HOST_ID_INCONSISTENT_FORMAT),
	SC_ENTRY(KEEP_ALIVE_TIMEOUT_EXPIRED),
	SC_ENTRY(KEEP_ALIVE_TIMEOUT_INVALID),
	SC_ENTRY(ABORTED_DUE_TO_PREEMPT),
	SC_ENTRY(SANITIZE_FAILED),
	SC_ENTRY(SANITIZE_IN_PROGRESS),
	SC_ENTRY(SGL_DATA_BLOCK_GRAN_INVALID),
	SC_ENTRY(NOT_SUPPORTED_IN_CMB),
	SC_ENTRY(NAMESPACE_IS_WRITE_PROTECTED),
	SC_ENTRY(COMMAND_INTERRUPTED),
	SC_ENTRY(TRANSIENT_TRANSPORT_ERROR),

	SC_ENTRY(LBA_OUT_OF_RANGE),
	SC_ENTRY(CAPACITY_EXCEEDED),
	SC_ENTRY(NAMESPACE_NOT_READY),
	SC_ENTRY(RESERVATION_CONFLICT),
	SC_ENTRY(FORMAT_IN_PROGRESS),
};

static const char *command_specific_status[256] = {
	SC_ENTRY(COMPLETION_QUEUE_INVALID),
	SC_ENTRY(INVALID_QUEUE_IDENTIFIER),
	SC_ENTRY(MAXIMUM_QUEUE_SIZE_EXCEEDED),
	SC_ENTRY(ABORT_COMMAND_LIMIT_EXCEEDED),
	SC_ENTRY(ASYNC_EVENT_REQUEST_LIMIT_EXCEEDED),
	SC_ENTRY(INVALID_FIRMWARE_SLOT),
	SC_ENTRY(INVALID_FIRMWARE_IMAGE),
	SC_ENTRY(INVALID_INTERRUPT_VECTOR),
	SC_ENTRY(INVALID_LOG_PAGE),
	SC_ENTRY(INVALID_FORMAT),
	SC_ENTRY(FIRMWARE_REQUIRES_RESET),
	SC_ENTRY(INVALID_QUEUE_DELETION),
	SC_ENTRY(FEATURE_NOT_SAVEABLE),
	SC_ENTRY(FEATURE_NOT_CHANGEABLE),
	SC_ENTRY(FEATURE_NOT_NS_SPECIFIC),
	SC_ENTRY(FW_ACT_REQUIRES_NVMS_RESET),
	SC_ENTRY(FW_ACT_REQUIRES_RESET),
	SC_ENTRY(FW_ACT_REQUIRES_TIME),
	SC_ENTRY(FW_ACT_PROHIBITED),
	SC_ENTRY(OVERLAPPING_RANGE),
	SC_ENTRY(NS_INSUFFICIENT_CAPACITY),
	SC_ENTRY(NS_ID_UNAVAILABLE),
	SC_ENTRY(NS_ALREADY_ATTACHED),
	SC_ENTRY(NS_IS_PRIVATE),
	SC_ENTRY(NS_NOT_ATTACHED),
	SC_ENTRY(THIN_PROV_NOT_SUPPORTED),
	SC_ENTRY(CTRLR_LIST_INVALID),
	SC_ENTRY(SELF_TEST_IN_PROGRESS),
	SC_ENTRY(BOOT_PART_WRITE_PROHIB),
	SC_ENTRY(INVALID_CTRLR_ID),
	SC_ENTRY(INVALID_SEC_CTRLR_STATE),
	SC_ENTRY(INVALID_NUM_OF_CTRLR_RESRC),
	SC_ENTRY(INVALID_RESOURCE_ID),
	SC_ENTRY(SANITIZE_PROHIBITED_WPMRE),
	SC_ENTRY(ANA_GROUP_ID_INVALID),
	SC_ENTRY(ANA_ATTACH_FAILED),

	SC_ENTRY(CONFLICTING_ATTRIBUTES),
	SC_ENTRY(INVALID_PROTECTION_INFO),
	SC_ENTRY(ATTEMPTED_WRITE_TO_RO_PAGE),
};

static const char *media_error_status[256] = {
	SC_ENTRY(WRITE_FAULTS),
	SC_ENTRY(UNRECOVERED_READ_ERROR),
	SC_ENTRY(GUARD_CHECK_ERROR),
	SC_ENTRY(APPLICATION_TAG_CHECK_ERROR),
	SC_ENTRY(REFERENCE_TAG_CHECK_ERROR),
	SC_ENTRY(COMPARE_FAILURE),
	SC_ENTRY(ACCESS_DENIED),
	SC_ENTRY(DEALLOCATED_OR_UNWRITTEN),
};

static const char *path_related_status[256] = {
	SC_ENTRY(INTERNAL_PATH_ERROR),
	SC_ENTRY(ASYMMETRIC_ACCESS_PERSISTENT_LOSS),
	SC_ENTRY(ASYMMETRIC_ACCESS_INACCESSIBLE),
	SC_ENTRY(ASYMMETRIC_ACCESS_TRANSITION),
	SC_ENTRY(CONTROLLER_PATHING_ERROR),
	SC_ENTRY(HOST_PATHING_ERROR),
	SC_ENTRY(COMMAND_ABORTED_BY_HOST),
};

void
nvme_opcode_sbuf(bool admin, uint8_t opc, struct sbuf *sb)
{
	const char *s, *type;

	if (admin) {
		s = admin_opcode[opc];
		type = "ADMIN";
	} else {
		s = nvm_opcode[opc];
		type = "NVM";
	}
	if (s == NULL)
		sbuf_printf(sb, "%s (%02x)", type, opc);
	else
		sbuf_printf(sb, "%s (%02x)", s, opc);
}

void
nvme_sc_sbuf(const struct nvme_completion *cpl, struct sbuf *sb)
{
	const char *s, *type;
	uint16_t status, sc, sct;

	status = le16toh(cpl->status);
	sc = NVME_STATUS_GET_SC(status);
	sct = NVME_STATUS_GET_SCT(status);
	switch (sct) {
	case NVME_SCT_GENERIC:
		s = generic_status[sc];
		type = "GENERIC";
		break;
	case NVME_SCT_COMMAND_SPECIFIC:
		s = command_specific_status[sc];
		type = "COMMAND SPECIFIC";
		break;
	case NVME_SCT_MEDIA_ERROR:
		s = media_error_status[sc];
		type = "MEDIA ERROR";
		break;
	case NVME_SCT_PATH_RELATED:
		s = path_related_status[sc];
		type = "PATH RELATED";
		break;
	case NVME_SCT_VENDOR_SPECIFIC:
		s = NULL;
		type = "VENDOR SPECIFIC";
		break;
	default:
		s = NULL;
		type = NULL;
		break;
	}

	if (type == NULL)
		sbuf_printf(sb, "RESERVED (%02x/%02x)", sct, sc);
	else if (s == NULL)
		sbuf_printf(sb, "%s (%02x/%02x)", type, sct, sc);
	else
		sbuf_printf(sb, "%s (%02x/%02x)", s, sct, sc);
}

void
nvme_cpl_sbuf(const struct nvme_completion *cpl, struct sbuf *sb)
{
	uint16_t status;

	status = le16toh(cpl->status);
	nvme_sc_sbuf(cpl, sb);
	if (NVME_STATUS_GET_M(status) != 0)
		sbuf_printf(sb, " M");
	if (NVME_STATUS_GET_DNR(status) != 0)
		sbuf_printf(sb, " DNR");
}


/* --- ctl_ser_table.c --- */
#define	pS	CTL_SER_PASS		/* Pass */
#define	bK	CTL_SER_BLOCK		/* Blocked */
#define	bO	CTL_SER_BLOCKOPT	/* Optional block */
#define	xT	CTL_SER_EXTENT		/* Extent check */
#define	xO	CTL_SER_EXTENTOPT	/* Optional extent check */
#define	xS	CTL_SER_SEQ		/* Sequential check */

const uint8_t
ctl_serialize_table[CTL_SERIDX_COUNT][CTL_SERIDX_COUNT] = {
/**>IDX_ :: 2nd:TUR RD  WRT UNM SYN MDSN MDSL RQSN INQ RDCP RES LSNS FMT STR*/
/*TUR     */{   pS, pS, pS, pS, pS, bK,  bK,  pS,  pS, pS,  bK, pS,  pS, bK},
/*READ    */{   pS, xS, xT, xO, pS, bK,  bK,  pS,  pS, pS,  bK, pS,  bK, bK},
/*WRITE   */{   pS, xT, xT, xO, pS, bK,  bK,  pS,  pS, pS,  bK, pS,  bK, bK},
/*UNMAP   */{   pS, bO, bO, pS, pS, bK,  bK,  pS,  pS, pS,  bK, pS,  bK, bK},
/*SYNC    */{   pS, pS, bO, pS, pS, bK,  bK,  pS,  pS, pS,  bK, pS,  bK, bK},
/*MD_SNS  */{   bK, bK, bK, bK, bK, pS,  bK,  pS,  pS, pS,  bK, pS,  bK, bK},
/*MD_SEL  */{   bK, bK, bK, bK, bK, bK,  bK,  pS,  pS, pS,  bK, bK,  bK, bK},
/*RQ_SNS  */{   bK, bK, bK, bK, bK, bK,  bK,  bK,  bK, bK,  bK, bK,  pS, bK},
/*INQ     */{   pS, pS, pS, pS, pS, pS,  pS,  pS,  pS, pS,  pS, pS,  pS, pS},
/*RD_CAP  */{   pS, pS, pS, pS, pS, pS,  pS,  pS,  pS, pS,  bK, pS,  bK, bK},
/*RES     */{   bK, bK, bK, bK, bK, bK,  bK,  bK,  pS, pS,  bK, bK,  bK, bK},
/*LOG_SNS */{   pS, pS, pS, pS, pS, pS,  pS,  pS,  pS, pS,  bK, pS,  bK, bK},
/*FORMAT  */{   bK, bK, bK, bK, bK, bK,  bK,  bK,  bK, bK,  bK, bK,  bK, bK},
/*START   */{   bK, bK, bK, bK, bK, bK,  bK,  bK,  bK, pS,  bK, bK,  bK, bK},
};


/* --- ctl_nvme_cmd_table.c --- */
/* Administrative Command Set (CTL_IO_NVME_ADMIN). */
const struct ctl_nvme_cmd_entry nvme_admin_cmd_table[256] =
{
	[NVME_OPC_IDENTIFY] = { ctl_nvme_identify, CTL_FLAG_DATA_IN |
				CTL_CMD_FLAG_OK_ON_NO_LUN },
};

/* NVM Command Set (CTL_IO_NVME). */
const struct ctl_nvme_cmd_entry nvme_nvm_cmd_table[256] =
{
	[NVME_OPC_FLUSH] = { ctl_nvme_flush, CTL_FLAG_DATA_NONE },
	[NVME_OPC_WRITE] = { ctl_nvme_read_write, CTL_FLAG_DATA_OUT },
	[NVME_OPC_READ] = { ctl_nvme_read_write, CTL_FLAG_DATA_IN },
	[NVME_OPC_WRITE_UNCORRECTABLE] = { ctl_nvme_write_uncorrectable,
					   CTL_FLAG_DATA_NONE },
	[NVME_OPC_COMPARE] = { ctl_nvme_compare, CTL_FLAG_DATA_OUT },
	[NVME_OPC_WRITE_ZEROES] = { ctl_nvme_write_zeroes, CTL_FLAG_DATA_NONE },
	[NVME_OPC_DATASET_MANAGEMENT] = { ctl_nvme_dataset_management,
					  CTL_FLAG_DATA_OUT },
	[NVME_OPC_VERIFY] = { ctl_nvme_verify, CTL_FLAG_DATA_NONE },
};

/* --- ctl_nvme_all.c --- */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2012-2014 Intel Corporation
 * All rights reserved.
 *
 * Copyright (c) 2023 Chelsio Communications, Inc.
 */

#ifndef _KERNEL
#endif



void
ref_ctl_nvme_command_string(struct ctl_nvmeio *ctnio, struct sbuf *sb)
{
	nvme_opcode_sbuf(ctnio->io_hdr.io_type == CTL_IO_NVME_ADMIN,
	    ctnio->cmd.opc, sb);
}

void
ref_ctl_nvme_status_string(struct ctl_nvmeio *ctnio, struct sbuf *sb)
{
	nvme_cpl_sbuf(&ctnio->cpl, sb);
}
/* --- ctl_backend.c --- */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2003 Silicon Graphics International Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: //depot/users/kenm/FreeBSD-test2/sys/cam/ctl/ctl_backend.c#3 $
 */
/*
 * CTL backend driver registration routines
 *
 * Author: Ken Merry <ken@FreeBSD.org>
 */



int
ref_ctl_backend_register(struct ctl_backend_driver *be)
{
	struct ctl_softc *softc = control_softc;
	struct ctl_backend_driver *be_tmp;
	int error;

	/* Sanity check, make sure this isn't a duplicate registration. */
	mtx_lock(&softc->ctl_lock);
	STAILQ_FOREACH(be_tmp, &softc->be_list, links) {
		if (strcmp(be_tmp->name, be->name) == 0) {
			mtx_unlock(&softc->ctl_lock);
			return (-1);
		}
	}
	mtx_unlock(&softc->ctl_lock);
#ifdef CS_BE_CONFIG_MOVE_DONE_IS_NOT_USED
	be->config_move_done = ctl_config_move_done;
#endif

	/* Call the backend's initialization routine. */
	if (be->init != NULL) {
		if ((error = be->init()) != 0) {
			printf("%s backend init error: %d\n",
			    be->name, error);
			return (error);
		}
	}

	mtx_lock(&softc->ctl_lock);
	STAILQ_INSERT_TAIL(&softc->be_list, be, links);
	softc->num_backends++;
	mtx_unlock(&softc->ctl_lock);
	return (0);
}

int
ref_ctl_backend_deregister(struct ctl_backend_driver *be)
{
	struct ctl_softc *softc = control_softc;
	int error;

	/* Call the backend's shutdown routine. */
	if (be->shutdown != NULL) {
		if ((error = be->shutdown()) != 0) {
			printf("%s backend shutdown error: %d\n",
			    be->name, error);
			return (error);
		}
	}

	mtx_lock(&softc->ctl_lock);
	STAILQ_REMOVE(&softc->be_list, be, ctl_backend_driver, links);
	softc->num_backends--;
	mtx_unlock(&softc->ctl_lock);
	return (0);
}

struct ctl_backend_driver *
ref_ctl_backend_find(char *backend_name)
{
	struct ctl_softc *softc = control_softc;
	struct ctl_backend_driver *be_tmp;

	mtx_lock(&softc->ctl_lock);
	STAILQ_FOREACH(be_tmp, &softc->be_list, links) {
		if (strcmp(be_tmp->name, backend_name) == 0) {
			mtx_unlock(&softc->ctl_lock);
			return (be_tmp);
		}
	}
	mtx_unlock(&softc->ctl_lock);

	return (NULL);
}

void ref_reset_backend_state(void)
{
	memset(&ref_ctl_softc_storage, 0, sizeof(ref_ctl_softc_storage));
	STAILQ_INIT(&ref_ctl_softc_storage.be_list);
}

uint32_t ref_num_backends(void)
{
	return control_softc->num_backends;
}

