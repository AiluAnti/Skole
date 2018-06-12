#include <stdlib.h>
#include <stdint.h>
#include "transport_layer.h"
#include "transport_package_impl.h"
#include "diagnostics.h"
#include "list.h"

static void transport_layer_timer_set(transport_layer_t* tp_layer, tick_timer_t* timer, int ticks);

struct transport_layer
{
	osi_stack_t* osi_stack;

	// STUDENTS BEGIN:
	// Extend this structure with your own status variables for the transport layer.
	uint8_t ack;
	uint8_t seq;
	uint16_t pkg_cnt;
	transport_package_t* rcv_pkg;
	transport_package_t* snd_pkg;
	tick_timer_t* timer;
	list_t *queue;
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

	tp_layer = (transport_layer_t*)malloc(sizeof(transport_layer_t));
	if (!tp_layer)
		return NULL;

	DIAG_PRINTF(DIAG_DEBUG, osi_stack, LAYER_TP, "Creating transport layer %#p", tp_layer);
	tp_layer->osi_stack = osi_stack;
	tp_layer->ack = 0;
	tp_layer->seq = 0;
	tp_layer->rcv_pkg = NULL;
	tp_layer->snd_pkg = NULL;
	tp_layer->pkg_cnt = 0;
	tp_layer->timer = (tick_timer_t*)calloc(sizeof(tick_timer_t), 1);
	tp_layer->queue = list_create();

	transport_layer_timer_set(tp_layer, tp_layer->timer, 10000);

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

void print_pkg(transport_package_t *pkg)
{
	int i = 0;
/*
	printf("PKG: %p, ack %d, seq %d, size %d, data: ", pkg, pkg->ack, pkg->seq, pkg->size);
	for(; i < pkg->size; i++)
		printf(" %d,", ((unsigned char*)pkg->data)[i]);
	puts("");*/
}

void print_layer(transport_layer_t* tp_layer)
{
	printf("layer: %p, ack: %d, seq: %d\n", tp_layer, tp_layer->ack, tp_layer->seq);
}


void transport_layer_onAppSend(transport_layer_t* tp_layer, void* data, size_t size)
{
	tp_layer->seq = !(tp_layer->seq);	
	transport_package_t* pkg = transport_pkg_create(data, size, tp_layer->ack, tp_layer->seq);
	if (!tp_layer->snd_pkg) // If there is no pkg waiting to be acked
	{
		// Creating a transport pkg to send	

		tp_layer->snd_pkg = transport_pkg_copy(pkg); // This pkg will be the last sent one
		
		// Print packet and layer info
		//print_pkg(pkg);
		//print_layer(tp_layer);
		
		DIAG_PRINTF(DIAG_DEBUG, tp_layer->osi_stack, LAYER_TP, "Starting timer");
		DIAG_PRINTF(DIAG_CRITICAL, tp_layer->osi_stack, LAYER_TP, "Sending package to nw layer, SEQ:", pkg->seq);
		osi_tp2nw(tp_layer->osi_stack, pkg);
	}
	else // If we have a pkg in the air
	{
		list_addlast(tp_layer->queue, transport_pkg_copy(pkg));
		//printf("PACKET IN AIR: Size of queue %p is: %d\n", tp_layer->queue, list_size(tp_layer->queue));
	}	
}

void transport_layer_onNwReceive(transport_layer_t* tp_layer, transport_package_t* tp_pkg)
{
	print_pkg(tp_pkg);
	print_layer(tp_layer);
	if (tp_pkg->size && (tp_pkg->seq != tp_layer->ack)) // If a regular data package is received
	{
		tp_layer->pkg_cnt++;
		DIAG_PRINTF(DIAG_CRITICAL, tp_layer->osi_stack, LAYER_TP, "RECIPIENT: PKG CNT RECEIVED: %d", tp_layer->pkg_cnt);
		
		// Print pkg and layer info
		osi_tp2app(tp_layer->osi_stack, tp_pkg->data, tp_pkg->size);
		tp_layer->ack = !(tp_layer->ack);
		tp_layer->rcv_pkg = tp_pkg;

		transport_package_t* ack_pkg = transport_pkg_create(NULL, 0, tp_pkg->seq, 0);
		DIAG_PRINTF(DIAG_DEBUG, tp_layer->osi_stack, LAYER_TP, "Sending ACK to NW layer");		
		osi_tp2nw(tp_layer->osi_stack, ack_pkg);
		return;
	}
	else if(tp_pkg->size && (tp_pkg->seq == tp_layer->ack)) // Retransmit
	{
		DIAG_PRINTF(DIAG_CRITICAL, tp_layer->osi_stack, LAYER_TP, "RETRANSMIT ACK");
		transport_package_t* ack_pkg = transport_pkg_create(NULL, 0, tp_layer->rcv_pkg->seq, 0);
		DIAG_PRINTF(DIAG_DEBUG, tp_layer->osi_stack, LAYER_TP, "Sending ACK to NW layer");		
		osi_tp2nw(tp_layer->osi_stack, ack_pkg);

	}

	if (!tp_pkg->size && (tp_pkg->ack == tp_layer->snd_pkg->seq)) // If a correct ack is received
	{
		DIAG_PRINTF(DIAG_CRITICAL, tp_layer->osi_stack, LAYER_TP, "SENDER: Freeing send packet");
		//transport_pkg_destroy(tp_layer->snd_pkg);
		tp_layer->snd_pkg = NULL;
		if (list_size(tp_layer->queue) >= 1) // If there are items in the queue
		{
			printf("NO PACKETS IN AIR: The size of the queue is: %d\n", list_size(tp_layer->queue));
			DIAG_PRINTF(DIAG_DEBUG, tp_layer->osi_stack, LAYER_TP, "SENDER: Sending from the queue");
			transport_package_t* pkg = list_popfirst(tp_layer->queue);
			tp_layer->snd_pkg = transport_pkg_copy(pkg);



			print_pkg(pkg);
			print_layer(tp_layer);

			DIAG_PRINTF(DIAG_DEBUG, tp_layer->osi_stack, LAYER_TP, "Starting timer");
			DIAG_PRINTF(DIAG_DEBUG, tp_layer->osi_stack, LAYER_TP, "SENDER: Sending package to nw layer");
			osi_tp2nw(tp_layer->osi_stack, pkg);
		}

	}
	else if(!tp_pkg->size && (tp_pkg->ack != tp_layer->snd_pkg->seq))
	{
		DIAG_PRINTF(DIAG_CRITICAL, tp_layer->osi_stack, LAYER_TP, "WRONG ACK OR WRONG PACKAGE");

	}

}

void transport_layer_onLayerTimeout(transport_layer_t* tp_layer)
{
	DIAG_PRINTF(DIAG_DEBUG, tp_layer->osi_stack, LAYER_TP, "TIMEOUT");
	if (tp_layer->snd_pkg)
	{
		transport_layer_timer_set(tp_layer, tp_layer->timer, 10000);
		osi_tp2nw(tp_layer->osi_stack, tp_layer->snd_pkg);
	}
	else if (!(tp_layer->snd_pkg))
		DIAG_PRINTF(DIAG_CRITICAL, tp_layer->osi_stack, LAYER_TP, "tp_layer->snd_pkg does not exist");
	

	//transport_package_t* ack_pkg = transport_pkg_create(NULL, 0, tp_layer->ack, 0);		
	//osi_tp2nw(tp_layer->osi_stack, ack_pkg);

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
