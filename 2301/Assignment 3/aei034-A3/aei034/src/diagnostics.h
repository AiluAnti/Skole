#pragma once
#ifndef DIAGNOSTICS_H
/// <summary>Header inclusion macro.</summary>
#define DIAGNOSTICS_H 1

#include "osi.h"

typedef enum diag_trace_lvl
{
	DIAG_CRITICAL = 0,
	DIAG_ERROR = 1,
	DIAG_WARNING = 2,
	DIAG_INFO = 3,
	DIAG_VERBOSE = 4,
	DIAG_DEBUG = 5
} diag_trace_lvl_t;

typedef enum diag_trace_src
{
	LAYER_NONE = 0,
	LAYER_NW = 3,
	LAYER_TP = 4,
	LAYER_APP = 5
} diag_trace_src_t;

extern int TRACE_LEVEL;

int DIAG_PRINTF(diag_trace_lvl_t severity, osi_stack_t* osi_stack, diag_trace_src_t layer, const char fmt[], ...);

#endif // !DIAGNOSTICS_H
