#include <stdlib.h>

#include "osi_impl.h"

void osi_app2tp(osi_stack_t* osi_stack, void* data, size_t size)
{
	if (!osi_stack)
		return;

	transport_layer_onAppSend(osi_stack->tp_layer, data, size);
}

void osi_tp2nw(osi_stack_t* osi_stack, transport_package_t* tp_pkg)
{
	if (!osi_stack)
		return;

	network_layer_onTpSend(osi_stack->nw_layer, tp_pkg);
}

void osi_nw2tp(osi_stack_t* osi_stack, transport_package_t* tp_pkg)
{
	if (!osi_stack)
		return;

	transport_layer_onNwReceive(osi_stack->tp_layer, tp_pkg);
}

void osi_tp2app(osi_stack_t* osi_stack, void* data, size_t size)
{
	if (!osi_stack)
		return;

	application_layer_onTpReceive(osi_stack->app_layer, data, size);
}

void osi_stack_init(osi_stack_t* osi_stack)
{
	if (!osi_stack)
		return;

	network_layer_init(osi_stack->nw_layer);
	transport_layer_init(osi_stack->tp_layer);
	application_layer_init(osi_stack->app_layer);
}

void osi_stack_teardown(osi_stack_t* osi_stack)
{
	if (!osi_stack)
		return;

	network_layer_onTeardown(osi_stack->nw_layer);
	transport_layer_onTeardown(osi_stack->tp_layer);
	application_layer_onTeardown(osi_stack->app_layer);
}
