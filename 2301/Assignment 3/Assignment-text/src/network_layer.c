#include <memory.h>
#include <stdlib.h>

#include "diagnostics.h"
#include "randomlib.h"
#include "timer.h"
#include "network_layer_impl.h"

network_layer_t* network_layer_create(osi_stack_t* osi_stack)
{
	network_layer_t* nw_layer;

	if (!osi_stack)
		return NULL;

	nw_layer = (network_layer_t*)malloc(sizeof(network_layer_t));
	if (!nw_layer)
		return NULL;
	DIAG_PRINTF(DIAG_DEBUG, osi_stack, LAYER_NW, "Creating network layer %#p", nw_layer);

	nw_layer->osi_stack = osi_stack;

	return nw_layer;
}

void network_layer_destroy(network_layer_t* nw_layer)
{
	if (nw_layer)
	{
		DIAG_PRINTF(DIAG_DEBUG, nw_layer->osi_stack, LAYER_NW, "Freeing network layer %#p", nw_layer);
		free(nw_layer);
	}
}

void network_layer_init(network_layer_t* nw_layer)
{
	if (!nw_layer)
		return;
}

void network_layer_onTeardown(network_layer_t* nw_layer)
{
	if (!nw_layer)
		return;
	DIAG_PRINTF(DIAG_DEBUG, nw_layer->osi_stack, LAYER_NW, "Layer being torn down: %#p", nw_layer);

	nw_layer->transmit_callback = NULL;
}

void network_layer_onTpSend(network_layer_t* nw_layer, transport_package_t* tp_pkg)
{
	size_t i, s;
	int ticks, noCorrupt, noLoss;
	char* pkg_data;
	void** ctx;
	tick_timer_t* t;
	transport_package_t* tp_pkg_cpy;

	if (!nw_layer || !tp_pkg)
		return;
	DIAG_PRINTF(DIAG_INFO, nw_layer->osi_stack, LAYER_NW, "Outgoing transport package.");

	t = (tick_timer_t*)malloc(sizeof(tick_timer_t));
	DIAG_PRINTF(DIAG_DEBUG, nw_layer->osi_stack, LAYER_NW, "Created network transmission timer %#p for package transport.", t);

	noCorrupt = RandomInt(0, 10000) > (nw_layer->corrupt_prob * 10000);
	tp_pkg_cpy = transport_pkg_copy(tp_pkg);
	DIAG_PRINTF(DIAG_DEBUG, nw_layer->osi_stack, LAYER_NW, "Created transport package copy %#p for transmission.", tp_pkg_cpy);
	if (!noCorrupt)
	{
		DIAG_PRINTF(DIAG_WARNING, nw_layer->osi_stack, LAYER_NW, "Corrupting transport package.");
		for (i = 0U, s = transport_pkg_size(tp_pkg_cpy), pkg_data = (char*)transport_pkg_data(tp_pkg_cpy); i < s; i++)
		{
			pkg_data[i] ^= (char)RandomInt(0, 255);
		}
	}

	ctx = (void**)malloc(sizeof(void*) * 3);
	DIAG_PRINTF(DIAG_DEBUG, nw_layer->osi_stack, LAYER_NW, "Created event context %#p.", ctx);
	if (ctx)
	{
		ctx[0] = nw_layer->remote_stack;
		ctx[1] = t;
		ctx[2] = tp_pkg_cpy;
	}

	noLoss = RandomInt(0, 10000) > (nw_layer->loss_prob * 10000);
	if (noLoss)
	{
		ticks = (int)RandomGaussian(nw_layer->nw_delay, nw_layer->nw_stddev);
		timer_set(t, ticks, (void (*)(void *))nw_layer->transmit_callback, ctx);
	}
	else
	{
		DIAG_PRINTF(DIAG_WARNING, nw_layer->osi_stack, LAYER_NW, "Transport package %#p lost.");
		DIAG_PRINTF(DIAG_DEBUG, nw_layer->osi_stack, LAYER_NW, "Freeing network transmission timer %#p", t);
		free(t);
		DIAG_PRINTF(DIAG_DEBUG, nw_layer->osi_stack, LAYER_NW, "Freeing transport package copy %#p", t);
		transport_pkg_destroy(tp_pkg_cpy);
		DIAG_PRINTF(DIAG_DEBUG, nw_layer->osi_stack, LAYER_NW, "Freeing event context %#p", ctx);
		free(ctx);
	}
}
