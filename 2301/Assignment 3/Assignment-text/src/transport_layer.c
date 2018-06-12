#include <stdlib.h>

#include "transport_layer.h"
#include "transport_package_impl.h"
#include "diagnostics.h"

static void transport_layer_timer_set(transport_layer_t* tp_layer, tick_timer_t* timer, int ticks);

struct transport_layer
{
	osi_stack_t* osi_stack;

	// STUDENTS BEGIN:
	// Extend this structure with your own status variables for the transport layer.
	int ack;

	transport_package_t* rcv_pkg;
	transport_package_t* snd_pkg;

	tick_timer_t timer;

	int pkg_cnt;


	// STUDENTS END
};

// STUDENTS BEGIN:
// Implementation for the transport layer

transport_layer_t* transport_layer_create(osi_stack_t* osi_stack)
{
	// Remember to assign the osi_stack parameter to the new transport layer object you create in this function.
	transport_layer_t* tp_layer;

	if (!osi_stack)
		return NULL;

	tp_layer = (transport_layer_t*)malloc(sizeof(transport_layer_t))
	if (!tp_layer)
		return NULL;

	DIAG_PRINTF(DIAG_DEBUG, osi_stack, LAYER_TP, "Creating transport layer %#p", tp_layer);
	tp_layer->osi_stack = osi_stack;
	tp_layer->ack = 0;
	tp_layer->rcv_pkg = NULL;
	tp_layer->snd_pkg = NULL;
	tp_layer->pkg_cnt = 0;

	tp_layer->timer = (tick_timer_t*)malloc(sizeof(tick_timer_t), 1);

	return tp_layer;
}

void transport_layer_destroy(transport_layer_t* tp_layer)
{
	if (tp_layer)
	{
		DIAG_PRINTF(DIAG_DEBUG, tp_layer->osi_stack, LAYER_TP, "Freeing transport layer %#p", tp_layer);
		free(tp_layer);
	}
}

void transport_layer_onAppSend(transport_layer_t* tp_layer, void* data, size_t size)
{
	// 	This is where we handle what we send to the app_layer
	//  When app is sending
	if(!tp_layer || !data || !size)
		return;
	
	transport_layer_t* pkg = transport_layer_create(data, size, tp_layer->ack);
	cpy = transport_pkg_copy(pkg);
	tp_layer->snd_pkg = pkg;
	tp_layer->pkg_cnt += 1;
	osi_tp2nw(tp_layer->osi_stack, pkg);

}

void transport_layer_onNwReceive(transport_layer_t* tp_layer, transport_package_t* tp_pkg)
{
	/* 	This is where we handle what is received by the nw_layer
		When network sends to tp_layer
	*/
	if (!tp_layer || !tp_pkg->data)
		return;
	else if (!tp_pkg->size)
	{
		DIAG_PRINTF(DIAG_WARNING, tp_layer->osi_stack, LAYER_TP, "Received data package of length 0. Shutting down connection");
		osi_stack_teardown(tp_layer->osi_stack);
		return;
	}

	osi_tp2app(tp_layer->osi_stack, tp_pkg->data, tp_pkg->size);
	/*if (tp_layer->ack == tp_pkg->seqnr)
	{
		DIAG_PRINTF(DIAG_WARNING, tp_layer->osi_stack, LAYER_TP, "Received unexpected data, shutting down.");
		return;
	}
	else if(tp_layer->ack != tp_pkg->seqnr)
	{
		tp_layer->seqnr = ~tp_layer->seqnr;
		tp_layer->ack = ~tp_layer->ack;
	}*/

}

void transport_layer_onLayerTimeout(transport_layer_t* tp_layer)
{

}


// EXTRA PROBLEM:
// Establish connection by syncing the transmission (e.g. TCP SYN-packets)
// Shutdown connection when the application tears down the OSI stack

void transport_layer_init(transport_layer_t* tp_layer)
{
	if (!tp_layer)
		return;
}

void transport_layer_onTeardown(transport_layer_t* tp_layer)
{

}

// The following code defines convenience functions for you to use for timers.
// Feel free to look through it, but do not worry if you cannot make heads or tails of it.

static void transport_layer_timer_set(transport_layer_t* tp_layer, tick_timer_t* timer, int ticks)
{
	timer_set(timer, ticks, (void(*)(void*))transport_layer_onLayerTimeout, tp_layer);
}
