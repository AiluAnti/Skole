#include <stdio.h>
#include <stdarg.h>

#include "diagnostics.h"

static char* DIAG_SEVERITY_STRING(diag_trace_lvl_t severity)
{
	switch (severity)
	{
		case DIAG_CRITICAL:	return "CRITICAL";
		case DIAG_ERROR:	return "   ERROR";
		case DIAG_WARNING:	return " WARNING";
		case DIAG_INFO:		return "    INFO";
		case DIAG_VERBOSE:	return " VERBOSE";
		case DIAG_DEBUG:	return "   DEBUG";
		default:			return " UNKNOWN";
	}
}

static char* DIAG_LAYER_STRING(diag_trace_src_t layer)
{
	switch (layer)
	{
		case LAYER_NONE:	return "     NONE";
		case LAYER_NW:		return "  NETWORK";
		case LAYER_TP:		return "TRANSPORT";
		case LAYER_APP:		return "      APP";
		default:			return "  UNKNOWN";
	}
}

int DIAG_PRINTF(diag_trace_lvl_t severity, osi_stack_t* osi_stack, diag_trace_src_t layer, const char fmt[], ...)
{
	va_list argptr;
	int c = 0;
	if (severity > TRACE_LEVEL)
		return c;

	c += printf("[***%s***] {%#p} [%s] ", DIAG_SEVERITY_STRING(severity), osi_stack, DIAG_LAYER_STRING(layer));

	va_start(argptr, fmt);
	c += vprintf(fmt, argptr);
	c += puts("");
	return c;
}
