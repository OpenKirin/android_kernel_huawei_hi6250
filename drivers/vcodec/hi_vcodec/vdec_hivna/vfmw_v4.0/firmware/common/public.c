#ifdef ENV_ARMLINUX_KERNEL
#include "stdarg.h"
#endif

#include "public.h"
#ifdef ENV_ARMLINUX_KERNEL
OSAL_FILE *g_fpLog = NULL;
#endif

UINT32 g_PrintEnable = DEFAULT_PRINT_ENABLE;
UINT32 g_PrintDevice = DEFAULT_PRINT_DEVICE;
SINT8  g_TmpMsg[1024];

SINT32 g_TraceCtrl = 0xffffffff;/*lint !e569*/

#ifdef ENV_ARMLINUX_KERNEL
SINT32 dprint_linux_kernel(UINT32 type, const SINT8 *format, ...)
{
	va_list args;
	SINT32 nTotalChar = 0;

	if ((PRN_ALWS != type) && ((g_PrintEnable & (1 << type)) == 0))
		return -1;

	va_start(args, format);
	nTotalChar = vsnprintf(g_TmpMsg, sizeof(g_TmpMsg), format, args);
	va_end(args);

	if ((nTotalChar <= 0) || (nTotalChar >= 1023))
		return -1;

	if (DEV_SCREEN == g_PrintDevice) {
#ifndef  HI_ADVCA_FUNCTION_RELEASE
		return (VFMW_OSAL_Print(KERN_ALERT "%s", g_TmpMsg));
#else
		return 0;
#endif
	} else if (DEV_SYSLOG == g_PrintDevice) {

	} else if (DEV_FILE == g_PrintDevice) {
		if (g_fpLog)
			return (OSAL_FileWrite(g_TmpMsg, nTotalChar, g_fpLog));
	} else if (DEV_MEM == g_PrintDevice) {
	}

	return -1;
}
#endif
