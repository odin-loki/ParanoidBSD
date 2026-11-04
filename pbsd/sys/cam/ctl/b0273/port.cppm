module;

#include <cstdarg>
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>

export module pbsd.sys.cam.ctl.b0273;

export namespace pbsd::sys_cam_ctl::b0273 {


#ifndef LONG_BIT
#define LONG_BIT (8 * (int)sizeof(long))
#endif

#define le16toh(x) (x)
#define CTL_TIME_IO
#define CTL_NUM_PRIV 6
#define CTL_BE_NAME_LEN 32

#define QMD_STAILQ_CHECK_TAIL(head) do {} while (0)
#define QMD_SAVELINK(name, link) __typeof__(link) name = (link)
#define TRASHIT(x) do {(void)(x);} while (0)
#define QUEUE_TYPEOF(type) struct type

#define STAILQ_HEAD(name, type) struct name { struct type *stqh_first; struct type **stqh_last; }
#define STAILQ_ENTRY(type) struct { struct type *stqe_next; }
#define STAILQ_FIRST(head) ((head)->stqh_first)
#define STAILQ_NEXT(elm, field) ((elm)->field.stqe_next)
#define STAILQ_EMPTY(head) (STAILQ_FIRST(head) == nullptr)
#define STAILQ_INIT(head) do { STAILQ_FIRST((head)) = nullptr; (head)->stqh_last = &STAILQ_FIRST((head)); } while (0)
#define STAILQ_FOREACH(var, head, field) for ((var) = STAILQ_FIRST((head)); (var); (var) = STAILQ_NEXT((var), field))
#define STAILQ_INSERT_TAIL(head, elm, field) do { \
	__typeof__((head)->stqh_last) prevlast = (head)->stqh_last; \
	STAILQ_NEXT((elm), field) = nullptr; \
	(head)->stqh_last = &STAILQ_NEXT((elm), field); \
	*prevlast = (elm); \
} while (0)
#define STAILQ_REMOVE(head, elm, type, field) do { \
	QMD_SAVELINK(_Oldnext, (elm)->field.stqe_next); \
	if (STAILQ_FIRST((head)) == (elm)) { \
		if ((STAILQ_FIRST((head)) = STAILQ_NEXT(STAILQ_FIRST((head)), field)) == nullptr) \
			(head)->stqh_last = &STAILQ_FIRST((head)); \
	} else { \
		QUEUE_TYPEOF(type) *_Curelm = STAILQ_FIRST(head); \
		while (STAILQ_NEXT(_Curelm, field) == (elm)) \
			_Curelm = STAILQ_NEXT(_Curelm, field); \
		if ((STAILQ_NEXT(_Curelm, field) = STAILQ_NEXT(STAILQ_NEXT(_Curelm, field), field)) == nullptr) \
			(head)->stqh_last = &STAILQ_NEXT((_Curelm), field); \
	} \
	TRASHIT(*_Oldnext); \
} while (0)

using u_int = unsigned int;
using u_long = unsigned long;
using caddr_t = char *;
using ctl_io_flags = int;

enum ctl_io_type {
	CTL_IO_NONE,
	CTL_IO_SCSI,
	CTL_IO_TASK,
	CTL_IO_NVME,
	CTL_IO_NVME_ADMIN,
};

enum ctl_serialize_action {
	CTL_SER_SEQ,
	CTL_SER_PASS,
	CTL_SER_EXTENTOPT,
	CTL_SER_EXTENT,
	CTL_SER_BLOCKOPT,
	CTL_SER_BLOCK,
};

enum ctl_seridx_t {
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
};

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

enum ctl_msg_type { CTL_MSG_SERIALIZE };
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
using ctl_ref = void (*)(void *, int);
using ctl_be_move_done_t = int (*)(union ctl_io *, bool);
using ctl_io_cont = int (*)(union ctl_io *);
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

using be_init_t = int (*)();
using be_shutdown_t = int (*)();
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

struct ctl_softc port_ctl_softc_storage;
ctl_softc *control_softc = &port_ctl_softc_storage;

void mtx_lock(struct mtx *m) { (void)m; }
void mtx_unlock(struct mtx *m) { (void)m; }

int sbuf_printf(struct sbuf *sb, const char *fmt, ...)
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


static const char *admin_opcode_lookup(unsigned idx)
{
	switch (idx) {
	case NVME_OPC_DELETE_IO_SQ: return "DELETE_IO_SQ";
	case NVME_OPC_CREATE_IO_SQ: return "CREATE_IO_SQ";
	case NVME_OPC_GET_LOG_PAGE: return "GET_LOG_PAGE";
	case NVME_OPC_DELETE_IO_CQ: return "DELETE_IO_CQ";
	case NVME_OPC_CREATE_IO_CQ: return "CREATE_IO_CQ";
	case NVME_OPC_IDENTIFY: return "IDENTIFY";
	case NVME_OPC_ABORT: return "ABORT";
	case NVME_OPC_SET_FEATURES: return "SET_FEATURES";
	case NVME_OPC_GET_FEATURES: return "GET_FEATURES";
	case NVME_OPC_ASYNC_EVENT_REQUEST: return "ASYNC_EVENT_REQUEST";
	case NVME_OPC_NAMESPACE_MANAGEMENT: return "NAMESPACE_MANAGEMENT";
	case NVME_OPC_FIRMWARE_ACTIVATE: return "FIRMWARE_ACTIVATE";
	case NVME_OPC_FIRMWARE_IMAGE_DOWNLOAD: return "FIRMWARE_IMAGE_DOWNLOAD";
	case NVME_OPC_DEVICE_SELF_TEST: return "DEVICE_SELF_TEST";
	case NVME_OPC_NAMESPACE_ATTACHMENT: return "NAMESPACE_ATTACHMENT";
	case NVME_OPC_KEEP_ALIVE: return "KEEP_ALIVE";
	case NVME_OPC_DIRECTIVE_SEND: return "DIRECTIVE_SEND";
	case NVME_OPC_DIRECTIVE_RECEIVE: return "DIRECTIVE_RECEIVE";
	case NVME_OPC_VIRTUALIZATION_MANAGEMENT: return "VIRTUALIZATION_MANAGEMENT";
	case NVME_OPC_NVME_MI_SEND: return "NVME_MI_SEND";
	case NVME_OPC_NVME_MI_RECEIVE: return "NVME_MI_RECEIVE";
	case NVME_OPC_CAPACITY_MANAGEMENT: return "CAPACITY_MANAGEMENT";
	case NVME_OPC_LOCKDOWN: return "LOCKDOWN";
	case NVME_OPC_DOORBELL_BUFFER_CONFIG: return "DOORBELL_BUFFER_CONFIG";
	case NVME_OPC_FABRICS_COMMANDS: return "FABRICS_COMMANDS";
	case NVME_OPC_FORMAT_NVM: return "FORMAT_NVM";
	case NVME_OPC_SECURITY_SEND: return "SECURITY_SEND";
	case NVME_OPC_SECURITY_RECEIVE: return "SECURITY_RECEIVE";
	case NVME_OPC_SANITIZE: return "SANITIZE";
	case NVME_OPC_GET_LBA_STATUS: return "GET_LBA_STATUS";
	default: return nullptr;
	}
}

static const char *nvm_opcode_lookup(unsigned idx)
{
	switch (idx) {
	case NVME_OPC_FLUSH: return "FLUSH";
	case NVME_OPC_WRITE: return "WRITE";
	case NVME_OPC_READ: return "READ";
	case NVME_OPC_WRITE_UNCORRECTABLE: return "WRITE_UNCORRECTABLE";
	case NVME_OPC_COMPARE: return "COMPARE";
	case NVME_OPC_WRITE_ZEROES: return "WRITE_ZEROES";
	case NVME_OPC_DATASET_MANAGEMENT: return "DATASET_MANAGEMENT";
	case NVME_OPC_VERIFY: return "VERIFY";
	case NVME_OPC_RESERVATION_REGISTER: return "RESERVATION_REGISTER";
	case NVME_OPC_RESERVATION_REPORT: return "RESERVATION_REPORT";
	case NVME_OPC_RESERVATION_ACQUIRE: return "RESERVATION_ACQUIRE";
	case NVME_OPC_RESERVATION_RELEASE: return "RESERVATION_RELEASE";
	case NVME_OPC_COPY: return "COPY";
	default: return nullptr;
	}
}

static const char *generic_status_lookup(unsigned idx)
{
	switch (idx) {
	case NVME_SC_SUCCESS: return "SUCCESS";
	case NVME_SC_INVALID_OPCODE: return "INVALID_OPCODE";
	case NVME_SC_INVALID_FIELD: return "INVALID_FIELD";
	case NVME_SC_COMMAND_ID_CONFLICT: return "COMMAND_ID_CONFLICT";
	case NVME_SC_DATA_TRANSFER_ERROR: return "DATA_TRANSFER_ERROR";
	case NVME_SC_ABORTED_POWER_LOSS: return "ABORTED_POWER_LOSS";
	case NVME_SC_INTERNAL_DEVICE_ERROR: return "INTERNAL_DEVICE_ERROR";
	case NVME_SC_ABORTED_BY_REQUEST: return "ABORTED_BY_REQUEST";
	case NVME_SC_ABORTED_SQ_DELETION: return "ABORTED_SQ_DELETION";
	case NVME_SC_ABORTED_FAILED_FUSED: return "ABORTED_FAILED_FUSED";
	case NVME_SC_ABORTED_MISSING_FUSED: return "ABORTED_MISSING_FUSED";
	case NVME_SC_INVALID_NAMESPACE_OR_FORMAT: return "INVALID_NAMESPACE_OR_FORMAT";
	case NVME_SC_COMMAND_SEQUENCE_ERROR: return "COMMAND_SEQUENCE_ERROR";
	case NVME_SC_INVALID_SGL_SEGMENT_DESCR: return "INVALID_SGL_SEGMENT_DESCR";
	case NVME_SC_INVALID_NUMBER_OF_SGL_DESCR: return "INVALID_NUMBER_OF_SGL_DESCR";
	case NVME_SC_DATA_SGL_LENGTH_INVALID: return "DATA_SGL_LENGTH_INVALID";
	case NVME_SC_METADATA_SGL_LENGTH_INVALID: return "METADATA_SGL_LENGTH_INVALID";
	case NVME_SC_SGL_DESCRIPTOR_TYPE_INVALID: return "SGL_DESCRIPTOR_TYPE_INVALID";
	case NVME_SC_INVALID_USE_OF_CMB: return "INVALID_USE_OF_CMB";
	case NVME_SC_PRP_OFFET_INVALID: return "PRP_OFFET_INVALID";
	case NVME_SC_ATOMIC_WRITE_UNIT_EXCEEDED: return "ATOMIC_WRITE_UNIT_EXCEEDED";
	case NVME_SC_OPERATION_DENIED: return "OPERATION_DENIED";
	case NVME_SC_SGL_OFFSET_INVALID: return "SGL_OFFSET_INVALID";
	case NVME_SC_HOST_ID_INCONSISTENT_FORMAT: return "HOST_ID_INCONSISTENT_FORMAT";
	case NVME_SC_KEEP_ALIVE_TIMEOUT_EXPIRED: return "KEEP_ALIVE_TIMEOUT_EXPIRED";
	case NVME_SC_KEEP_ALIVE_TIMEOUT_INVALID: return "KEEP_ALIVE_TIMEOUT_INVALID";
	case NVME_SC_ABORTED_DUE_TO_PREEMPT: return "ABORTED_DUE_TO_PREEMPT";
	case NVME_SC_SANITIZE_FAILED: return "SANITIZE_FAILED";
	case NVME_SC_SANITIZE_IN_PROGRESS: return "SANITIZE_IN_PROGRESS";
	case NVME_SC_SGL_DATA_BLOCK_GRAN_INVALID: return "SGL_DATA_BLOCK_GRAN_INVALID";
	case NVME_SC_NOT_SUPPORTED_IN_CMB: return "NOT_SUPPORTED_IN_CMB";
	case NVME_SC_NAMESPACE_IS_WRITE_PROTECTED: return "NAMESPACE_IS_WRITE_PROTECTED";
	case NVME_SC_COMMAND_INTERRUPTED: return "COMMAND_INTERRUPTED";
	case NVME_SC_TRANSIENT_TRANSPORT_ERROR: return "TRANSIENT_TRANSPORT_ERROR";
	case NVME_SC_LBA_OUT_OF_RANGE: return "LBA_OUT_OF_RANGE";
	case NVME_SC_CAPACITY_EXCEEDED: return "CAPACITY_EXCEEDED";
	case NVME_SC_NAMESPACE_NOT_READY: return "NAMESPACE_NOT_READY";
	case NVME_SC_RESERVATION_CONFLICT: return "RESERVATION_CONFLICT";
	case NVME_SC_FORMAT_IN_PROGRESS: return "FORMAT_IN_PROGRESS";
	default: return nullptr;
	}
}

static const char *command_specific_status_lookup(unsigned idx)
{
	switch (idx) {
	case NVME_SC_COMPLETION_QUEUE_INVALID: return "COMPLETION_QUEUE_INVALID";
	case NVME_SC_INVALID_QUEUE_IDENTIFIER: return "INVALID_QUEUE_IDENTIFIER";
	case NVME_SC_MAXIMUM_QUEUE_SIZE_EXCEEDED: return "MAXIMUM_QUEUE_SIZE_EXCEEDED";
	case NVME_SC_ABORT_COMMAND_LIMIT_EXCEEDED: return "ABORT_COMMAND_LIMIT_EXCEEDED";
	case NVME_SC_ASYNC_EVENT_REQUEST_LIMIT_EXCEEDED: return "ASYNC_EVENT_REQUEST_LIMIT_EXCEEDED";
	case NVME_SC_INVALID_FIRMWARE_SLOT: return "INVALID_FIRMWARE_SLOT";
	case NVME_SC_INVALID_FIRMWARE_IMAGE: return "INVALID_FIRMWARE_IMAGE";
	case NVME_SC_INVALID_INTERRUPT_VECTOR: return "INVALID_INTERRUPT_VECTOR";
	case NVME_SC_INVALID_LOG_PAGE: return "INVALID_LOG_PAGE";
	case NVME_SC_INVALID_FORMAT: return "INVALID_FORMAT";
	case NVME_SC_FIRMWARE_REQUIRES_RESET: return "FIRMWARE_REQUIRES_RESET";
	case NVME_SC_INVALID_QUEUE_DELETION: return "INVALID_QUEUE_DELETION";
	case NVME_SC_FEATURE_NOT_SAVEABLE: return "FEATURE_NOT_SAVEABLE";
	case NVME_SC_FEATURE_NOT_CHANGEABLE: return "FEATURE_NOT_CHANGEABLE";
	case NVME_SC_FEATURE_NOT_NS_SPECIFIC: return "FEATURE_NOT_NS_SPECIFIC";
	case NVME_SC_FW_ACT_REQUIRES_NVMS_RESET: return "FW_ACT_REQUIRES_NVMS_RESET";
	case NVME_SC_FW_ACT_REQUIRES_RESET: return "FW_ACT_REQUIRES_RESET";
	case NVME_SC_FW_ACT_REQUIRES_TIME: return "FW_ACT_REQUIRES_TIME";
	case NVME_SC_FW_ACT_PROHIBITED: return "FW_ACT_PROHIBITED";
	case NVME_SC_OVERLAPPING_RANGE: return "OVERLAPPING_RANGE";
	case NVME_SC_NS_INSUFFICIENT_CAPACITY: return "NS_INSUFFICIENT_CAPACITY";
	case NVME_SC_NS_ID_UNAVAILABLE: return "NS_ID_UNAVAILABLE";
	case NVME_SC_NS_ALREADY_ATTACHED: return "NS_ALREADY_ATTACHED";
	case NVME_SC_NS_IS_PRIVATE: return "NS_IS_PRIVATE";
	case NVME_SC_NS_NOT_ATTACHED: return "NS_NOT_ATTACHED";
	case NVME_SC_THIN_PROV_NOT_SUPPORTED: return "THIN_PROV_NOT_SUPPORTED";
	case NVME_SC_CTRLR_LIST_INVALID: return "CTRLR_LIST_INVALID";
	case NVME_SC_SELF_TEST_IN_PROGRESS: return "SELF_TEST_IN_PROGRESS";
	case NVME_SC_BOOT_PART_WRITE_PROHIB: return "BOOT_PART_WRITE_PROHIB";
	case NVME_SC_INVALID_CTRLR_ID: return "INVALID_CTRLR_ID";
	case NVME_SC_INVALID_SEC_CTRLR_STATE: return "INVALID_SEC_CTRLR_STATE";
	case NVME_SC_INVALID_NUM_OF_CTRLR_RESRC: return "INVALID_NUM_OF_CTRLR_RESRC";
	case NVME_SC_INVALID_RESOURCE_ID: return "INVALID_RESOURCE_ID";
	case NVME_SC_SANITIZE_PROHIBITED_WPMRE: return "SANITIZE_PROHIBITED_WPMRE";
	case NVME_SC_ANA_GROUP_ID_INVALID: return "ANA_GROUP_ID_INVALID";
	case NVME_SC_ANA_ATTACH_FAILED: return "ANA_ATTACH_FAILED";
	case NVME_SC_CONFLICTING_ATTRIBUTES: return "CONFLICTING_ATTRIBUTES";
	case NVME_SC_INVALID_PROTECTION_INFO: return "INVALID_PROTECTION_INFO";
	case NVME_SC_ATTEMPTED_WRITE_TO_RO_PAGE: return "ATTEMPTED_WRITE_TO_RO_PAGE";
	default: return nullptr;
	}
}

static const char *media_error_status_lookup(unsigned idx)
{
	switch (idx) {
	case NVME_SC_WRITE_FAULTS: return "WRITE_FAULTS";
	case NVME_SC_UNRECOVERED_READ_ERROR: return "UNRECOVERED_READ_ERROR";
	case NVME_SC_GUARD_CHECK_ERROR: return "GUARD_CHECK_ERROR";
	case NVME_SC_APPLICATION_TAG_CHECK_ERROR: return "APPLICATION_TAG_CHECK_ERROR";
	case NVME_SC_REFERENCE_TAG_CHECK_ERROR: return "REFERENCE_TAG_CHECK_ERROR";
	case NVME_SC_COMPARE_FAILURE: return "COMPARE_FAILURE";
	case NVME_SC_ACCESS_DENIED: return "ACCESS_DENIED";
	case NVME_SC_DEALLOCATED_OR_UNWRITTEN: return "DEALLOCATED_OR_UNWRITTEN";
	default: return nullptr;
	}
}

static const char *path_related_status_lookup(unsigned idx)
{
	switch (idx) {
	case NVME_SC_INTERNAL_PATH_ERROR: return "INTERNAL_PATH_ERROR";
	case NVME_SC_ASYMMETRIC_ACCESS_PERSISTENT_LOSS: return "ASYMMETRIC_ACCESS_PERSISTENT_LOSS";
	case NVME_SC_ASYMMETRIC_ACCESS_INACCESSIBLE: return "ASYMMETRIC_ACCESS_INACCESSIBLE";
	case NVME_SC_ASYMMETRIC_ACCESS_TRANSITION: return "ASYMMETRIC_ACCESS_TRANSITION";
	case NVME_SC_CONTROLLER_PATHING_ERROR: return "CONTROLLER_PATHING_ERROR";
	case NVME_SC_HOST_PATHING_ERROR: return "HOST_PATHING_ERROR";
	case NVME_SC_COMMAND_ABORTED_BY_HOST: return "COMMAND_ABORTED_BY_HOST";
	default: return nullptr;
	}
}

void nvme_opcode_sbuf(bool admin, uint8_t opc, struct sbuf *sb)
{
	const char *s, *type;
	if (admin) { s = admin_opcode_lookup(opc); type = "ADMIN"; }
	else { s = nvm_opcode_lookup(opc); type = "NVM"; }
	if (s == nullptr) sbuf_printf(sb, "%s (%02x)", type, opc);
	else sbuf_printf(sb, "%s (%02x)", s, opc);
}

void nvme_sc_sbuf(const struct nvme_completion *cpl, struct sbuf *sb)
{
	const char *s, *type;
	uint16_t status, sc, sct;
	status = le16toh(cpl->status);
	sc = NVME_STATUS_GET_SC(status);
	sct = NVME_STATUS_GET_SCT(status);
	switch (sct) {
	case NVME_SCT_GENERIC: s = generic_status_lookup(sc); type = "GENERIC"; break;
	case NVME_SCT_COMMAND_SPECIFIC: s = command_specific_status_lookup(sc); type = "COMMAND SPECIFIC"; break;
	case NVME_SCT_MEDIA_ERROR: s = media_error_status_lookup(sc); type = "MEDIA ERROR"; break;
	case NVME_SCT_PATH_RELATED: s = path_related_status_lookup(sc); type = "PATH RELATED"; break;
	case NVME_SCT_VENDOR_SPECIFIC: s = nullptr; type = "VENDOR SPECIFIC"; break;
	default: s = nullptr; type = nullptr; break;
	}
	if (type == nullptr) sbuf_printf(sb, "RESERVED (%02x/%02x)", sct, sc);
	else if (s == nullptr) sbuf_printf(sb, "%s (%02x/%02x)", type, sct, sc);
	else sbuf_printf(sb, "%s (%02x/%02x)", s, sct, sc);
}

void nvme_cpl_sbuf(const struct nvme_completion *cpl, struct sbuf *sb)
{
	uint16_t status = le16toh(cpl->status);
	nvme_sc_sbuf(cpl, sb);
	if (NVME_STATUS_GET_M(status) != 0) sbuf_printf(sb, " M");
	if (NVME_STATUS_GET_DNR(status) != 0) sbuf_printf(sb, " DNR");
}

struct ctl_nvmeio;
int ctl_nvme_identify(struct ctl_nvmeio *ctnio);
int ctl_nvme_flush(struct ctl_nvmeio *ctnio);
int ctl_nvme_read_write(struct ctl_nvmeio *ctnio);
int ctl_nvme_write_uncorrectable(struct ctl_nvmeio *ctnio);
int ctl_nvme_compare(struct ctl_nvmeio *ctnio);
int ctl_nvme_write_zeroes(struct ctl_nvmeio *ctnio);
int ctl_nvme_dataset_management(struct ctl_nvmeio *ctnio);
int ctl_nvme_verify(struct ctl_nvmeio *ctnio);

static ctl_nvme_cmd_entry nvme_admin_cmd_table_storage[256];
static ctl_nvme_cmd_entry nvme_nvm_cmd_table_storage[256];
ctl_nvme_cmd_entry *nvme_admin_cmd_table = nvme_admin_cmd_table_storage;
ctl_nvme_cmd_entry *nvme_nvm_cmd_table = nvme_nvm_cmd_table_storage;

static void init_nvme_cmd_tables()
{
	std::memset(nvme_admin_cmd_table_storage, 0, sizeof(nvme_admin_cmd_table_storage));
	std::memset(nvme_nvm_cmd_table_storage, 0, sizeof(nvme_nvm_cmd_table_storage));
	nvme_admin_cmd_table_storage[NVME_OPC_IDENTIFY] = { ctl_nvme_identify, CTL_FLAG_DATA_IN | CTL_CMD_FLAG_OK_ON_NO_LUN };
	nvme_nvm_cmd_table_storage[NVME_OPC_FLUSH] = { ctl_nvme_flush, CTL_FLAG_DATA_NONE };
	nvme_nvm_cmd_table_storage[NVME_OPC_WRITE] = { ctl_nvme_read_write, CTL_FLAG_DATA_OUT };
	nvme_nvm_cmd_table_storage[NVME_OPC_READ] = { ctl_nvme_read_write, CTL_FLAG_DATA_IN };
	nvme_nvm_cmd_table_storage[NVME_OPC_WRITE_UNCORRECTABLE] = { ctl_nvme_write_uncorrectable, CTL_FLAG_DATA_NONE };
	nvme_nvm_cmd_table_storage[NVME_OPC_COMPARE] = { ctl_nvme_compare, CTL_FLAG_DATA_OUT };
	nvme_nvm_cmd_table_storage[NVME_OPC_WRITE_ZEROES] = { ctl_nvme_write_zeroes, CTL_FLAG_DATA_NONE };
	nvme_nvm_cmd_table_storage[NVME_OPC_DATASET_MANAGEMENT] = { ctl_nvme_dataset_management, CTL_FLAG_DATA_OUT };
	nvme_nvm_cmd_table_storage[NVME_OPC_VERIFY] = { ctl_nvme_verify, CTL_FLAG_DATA_NONE };
}
struct nvme_cmd_table_init { nvme_cmd_table_init() { init_nvme_cmd_tables(); } };
static nvme_cmd_table_init nvme_cmd_table_init_obj;


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


int ctl_nvme_identify(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_flush(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_read_write(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_write_uncorrectable(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_compare(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_write_zeroes(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_dataset_management(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
int ctl_nvme_verify(struct ctl_nvmeio *ctnio) { (void)ctnio; return 0; }
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
ctl_nvme_command_string(struct ctl_nvmeio *ctnio, struct sbuf *sb)
{
	nvme_opcode_sbuf(ctnio->io_hdr.io_type == CTL_IO_NVME_ADMIN,
	    ctnio->cmd.opc, sb);
}

void
ctl_nvme_status_string(struct ctl_nvmeio *ctnio, struct sbuf *sb)
{
	nvme_cpl_sbuf(&ctnio->cpl, sb);
}
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
ctl_backend_register(struct ctl_backend_driver *be)
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
ctl_backend_deregister(struct ctl_backend_driver *be)
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
ctl_backend_find(char *backend_name)
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

void reset_backend_state()
{
	std::memset(&port_ctl_softc_storage, 0, sizeof(port_ctl_softc_storage));
	STAILQ_INIT(&port_ctl_softc_storage.be_list);
}

const uint8_t (&ctl_serialize_table_ref())[CTL_SERIDX_COUNT][CTL_SERIDX_COUNT]
{
	return ctl_serialize_table;
}

uint32_t num_backends()
{
	return control_softc->num_backends;
}

} // namespace
