#include <asm/compiler.h>
#include <linux/arm-smccc.h>
/*
 * HHEE general communication and test driver
 *
 */

#define ARM_STD_HVC_VERSION         0x8400ff03
/*
 * Defines for HHEE HVC function ids.
 */
#define HHEE_HVC_VERSION			0xC9000000
#define HHEE_HVC_NOTIFY				0xC9000001

/*
 * Defines for el2 register read
 */
#define HHEE_HCR_EL2				0xC9000002
#define HHEE_VTTBR_EL2				0xC9000003

/*
 * Defines for security level settings
 */
#define HHEE_PROT_LVL				0xC9000004
#define HHEE_PERMISIVE				0xC9000005
#define HHEE_TEXT_BOUNDARIES		0xC9000006
#define HHEE_RET_TO_USER            0xC9000007

/*
 * Defines for logging functionality
 */
#define HHEE_INIT_LOGBUF			0xC9000008
#define HHEE_LOGBUF_INFO            0xC9000009
#define HHEE_CRASHLOG_INFO			0xC900000A
#define HHEE_MONITORLOG_INFO		0xC900000B

/*
 * Defines for psci
 */
#define PSCI_VERSION			0x84000000
#define PSCI_CPU_SUSPEND_AARCH32    0x84000001
#define PSCI_CPU_SUSPEND_AARCH64    0xc4000001
#define PSCI_CPU_OFF            0x84000002
#define PSCI_CPU_ON_AARCH32     0x84000003
#define PSCI_CPU_ON_AARCH64     0xc4000003
#define PSCI_AFFINITY_INFO_AARCH32  0x84000004
#define PSCI_AFFINITY_INFO_AARCH64  0xc4000004
#define PSCI_MIG_AARCH32        0x84000005
#define PSCI_MIG_AARCH64        0xc4000005
#define PSCI_MIG_INFO_TYPE      0x84000006
#define PSCI_MIG_INFO_UP_CPU_AARCH32    0x84000007
#define PSCI_MIG_INFO_UP_CPU_AARCH64    0xc4000007
#define PSCI_SYSTEM_OFF         0x84000008
#define PSCI_SYSTEM_RESET       0x84000009
#define PSCI_FEATURES           0x8400000A

#define HHEE_ENABLE		1
#define HHEE_DISABLE	0

/*struct for logbuf*/
struct circular_buffer {
	unsigned long size; /* Indicates the total size of the buffer */
	unsigned long start; /* Starting point of valid data in buffer */
	unsigned long end; /* First character which is empty (can be written to) */
	unsigned long overflow; /* Indicator whether buffer has overwritten itself */
	unsigned long virt_log_addr; /*Indicator the virtual addr of buffer*/
	unsigned long virt_log_size; /*Indicator the max size of buffer*/
	unsigned int inited;        /*Indicator the status of buffer*/
	unsigned int logtype;       /*Indicator the type of buffer*/
	char *buf;
};

/*enum for logtype*/
enum ltype {
	NORMAL_LOG,
	CRASH_LOG,
	MONITOR_LOG,
};

int hhee_logger_init(void);
int hhee_init_debugfs(void);
void hhee_cleanup_debugfs(void);

ssize_t hhee_copy_logs(char __user *buf, size_t count,
		       loff_t *offp, int logtpye);

struct arm_smccc_res _hhee_fn_smc(unsigned long function_id,
			unsigned long arg0, unsigned long arg1,
			unsigned long arg2);
struct arm_smccc_res _hhee_fn_hvc(unsigned long function_id,
			unsigned long arg0, unsigned long arg1,
			unsigned long arg2);
unsigned long hhee_fn_smc(unsigned long function_id,
			unsigned long arg0, unsigned long arg1,
			unsigned long arg2);
unsigned long hhee_fn_hvc(unsigned long function_id,
			unsigned long arg0, unsigned long arg1,
			unsigned long arg2);
