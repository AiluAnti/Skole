#pragma once
#ifndef OSI_IMPL_H
/// <summary>Header inclusion macro.</summary>
#define OSI_IMPL_H 1

#include "osi.h"
#include "application_layer.h"
#include "transport_layer.h"
#include "network_layer.h"

struct osi_stack
{
	application_layer_t* app_layer;
	transport_layer_t* tp_layer;
	network_layer_t* nw_layer;
};

#endif // !OSI_IMPL_H
