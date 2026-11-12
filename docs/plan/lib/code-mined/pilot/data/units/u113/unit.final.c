#include "prelude.h"
#undef _LINUX_SUSPEND_H
#define _LINUX_SUSPEND_H
#undef PM_SUSPEND_ON
#define PM_SUSPEND_ON		((__force suspend_state_t) 0)
#undef PM_SUSPEND_TO_IDLE
#define PM_SUSPEND_TO_IDLE	((__force suspend_state_t) 1)
#undef PM_SUSPEND_STANDBY
#define PM_SUSPEND_STANDBY	((__force suspend_state_t) 2)
#undef PM_SUSPEND_MEM
#define PM_SUSPEND_MEM		((__force suspend_state_t) 3)
#undef PM_SUSPEND_MIN
#define PM_SUSPEND_MIN		PM_SUSPEND_TO_IDLE
#undef PM_SUSPEND_MAX
#define PM_SUSPEND_MAX		((__force suspend_state_t) 4)
#undef PM_SUSPEND_FLAG_FW_SUSPEND
#define PM_SUSPEND_FLAG_FW_SUSPEND	BIT(0)
#undef PM_SUSPEND_FLAG_FW_RESUME
#define PM_SUSPEND_FLAG_FW_RESUME	BIT(1)
#undef PM_SUSPEND_FLAG_NO_PLATFORM
#define PM_SUSPEND_FLAG_NO_PLATFORM	BIT(2)
#undef suspend_valid_only_mem
#define suspend_valid_only_mem	NULL
#undef pm_suspend_target_state
#define pm_suspend_target_state	(PM_SUSPEND_ON)
#undef PM_HIBERNATION_PREPARE
#define PM_HIBERNATION_PREPARE	0x0001 
#undef PM_POST_HIBERNATION
#define PM_POST_HIBERNATION	0x0002 
#undef PM_SUSPEND_PREPARE
#define PM_SUSPEND_PREPARE	0x0003 
#undef PM_POST_SUSPEND
#define PM_POST_SUSPEND		0x0004 
#undef PM_RESTORE_PREPARE
#define PM_RESTORE_PREPARE	0x0005 
#undef PM_POST_RESTORE
#define PM_POST_RESTORE		0x0006 
#undef pr_fmt
#define pr_fmt(fmt) "PM: " fmt
#undef pm_print_times_enabled
#define pm_print_times_enabled	(false)
#undef pm_debug_messages_on
#define pm_debug_messages_on	(false)
#undef pm_pr_dbg
#define pm_pr_dbg(fmt, ...)  	__pm_pr_dbg(fmt, ##__VA_ARGS__)
#undef pm_deferred_pr_dbg
#define pm_deferred_pr_dbg(fmt, ...)  	__pm_deferred_pr_dbg(fmt, ##__VA_ARGS__)
/*@ requires true; ensures \result == 0; */
static inline unsigned int lock_system_sleep(void) { return 0; }
