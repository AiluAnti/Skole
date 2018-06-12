#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "randomlib.h"

#include "diagnostics.h"
#include "timer.h"
#include "osi_impl.h"
#include "network_layer_impl.h"
#include "application_layer_impl.h"

int TRACE_LEVEL;
int num_packets, bidirectional;
float loss_prob;
float corrupt_prob;
float gen_delay, nw_delay, nw_stddev;

static osi_stack_t a_stack;
static osi_stack_t b_stack;

static void osi_nw_layer_onDiagramTick(void* ctx[3])
{
	osi_stack_t* target;

	if (!ctx)
		return;

	target = (osi_stack_t*)ctx[0];
	DIAG_PRINTF(DIAG_DEBUG, target, LAYER_NW, "Freeing network layer transmission timer %#p", ctx[1]);
	free(ctx[1]);

	DIAG_PRINTF(DIAG_INFO, target, LAYER_NW, "Incoming network datagram. Handover to transport layer.");
	osi_nw2tp(target, (transport_package_t*)ctx[2]);
	DIAG_PRINTF(DIAG_DEBUG, target, LAYER_NW, "Freeing transport package %#p", ctx[2]);
	free(ctx[2]);
	DIAG_PRINTF(DIAG_DEBUG, target, LAYER_NW, "Freeing event context %#p", ctx);
	free(ctx);
}

static void osi_app_layer_onDataGenerateTick(osi_stack_t* osi_stack)
{
	application_data_node_t* dataNode;
	application_layer_t* remote_app_layer;
	char data[MAX_APP_DATA_SIZE];
	size_t i, size;
	int ticks;

	if (!osi_stack)
		return;

	size = RandomInt(1, MAX_APP_DATA_SIZE);
	DIAG_PRINTF(DIAG_INFO, osi_stack, LAYER_APP, "Generating app data: %Iu bytes", (int)size);
	for (i = 0U; i < size; i++)
	{
		data[i] = (char)RandomInt(0, 255);
	}

	remote_app_layer = osi_stack->nw_layer->remote_stack->app_layer;
	dataNode = (application_data_node_t*)malloc(sizeof(application_data_node_t));
	if (dataNode)
	{
		DIAG_PRINTF(DIAG_DEBUG, osi_stack, LAYER_APP, "Initializing app data node %#p", dataNode);
		dataNode->idx = 0;
		dataNode->size = size;
		memcpy(dataNode->data, data, size);
		dataNode->next = NULL;

		if (remote_app_layer->dataLast)
			remote_app_layer->dataLast->next = dataNode;
		else
			remote_app_layer->dataFirst = dataNode;
		remote_app_layer->dataLast = dataNode;
	}

	osi_app2tp(osi_stack, data, size);
	osi_stack->app_layer->dataGenerateCount += 1;

	if (osi_stack->app_layer->dataGenerateCount < num_packets)
	{
		ticks = (int)RandomGaussian(gen_delay, 200);
		DIAG_PRINTF(DIAG_VERBOSE, osi_stack, LAYER_APP, "Scheduling additional app data generation after %d ticks.", ticks);
		timer_set(&(osi_stack->app_layer->dataGenerateTimer), ticks, (void (*)(void *))osi_app_layer_onDataGenerateTick, osi_stack);
	}
	else
	{
		DIAG_PRINTF(DIAG_INFO, osi_stack, LAYER_APP, "Data generation limit reached, not generating any more data.");
	}
}

static int shouldContinue(void)
{
	int aContinue, bContinue;
	aContinue = a_stack.app_layer->dataGenerateCount < num_packets || b_stack.app_layer->dataFirst;
	if (!bidirectional)
		return aContinue;
	bContinue = b_stack.app_layer->dataGenerateCount < num_packets || a_stack.app_layer->dataFirst;
	return aContinue || bContinue;
}

int main()
{
	int loopFlag = 0;
	char yesNoFlag;

	printf("-----  Reliable Transfer Protocol Simulator -------- \n\n");
	do
	{
		printf("Perform bidirectional package transmissions [y/n]: ");
		if (scanf("%c", &yesNoFlag))
		{
			switch (yesNoFlag)
			{
				case 'y':
				case 'Y':
					bidirectional = 1;
					loopFlag = 1;
					break;
				case 'n':
				case 'N':
					bidirectional = 0;
					loopFlag = 1;
					break;
				default:
					continue;
			}
		}
	} while (!loopFlag);

	printf("Enter the number of messages to simulate: ");
	if (!scanf("%d", &num_packets))
		num_packets = 0;

	printf("Enter packet loss probability [enter 0.0 for no loss]: ");
	if (!scanf("%f", &loss_prob))
		loss_prob = 0.0f;

	printf("Enter packet corruption probability [0.0 for no corruption]: ");
	if (!scanf("%f", &corrupt_prob))
		corrupt_prob = 0.0f;

	do
	{
		printf("Enter average time between messages from sender's application layer [ >= 0.0]: ");
		if (!scanf("%f", &gen_delay))
			gen_delay = 0.0f;
	} while (gen_delay < 0);

	do
	{
		printf("Enter average network layer transmission delay [ >= 0.0]: ");
		if (!scanf("%f", &nw_delay))
			nw_delay = 0.0f;
	} while (nw_delay < 0);

	printf("Enter network layer transmission delay standard deviation [e.g. 200]: ");
	if (!scanf("%f", &nw_stddev))
		nw_stddev = 200;
	printf("Specify TRACING level:\n * 0 - CRITICAL\n * 1 - ERROR\n * 2 - WARNING\n * 3 - INFO\n * 4 - VERBOSE\n * 5 - DEBUG\nTracing level: ");
	if (!scanf("%d", &TRACE_LEVEL))
		TRACE_LEVEL = 0;

	puts("");

	// Initialise random number generator
	RandomInitialise(45932, 86518);

	DIAG_PRINTF(DIAG_INFO, &a_stack, LAYER_NONE, "Labeling endpint A as 0x%p.", &a_stack);
	DIAG_PRINTF(DIAG_INFO, &a_stack, LAYER_NONE, "Labeling endpint B as 0x%p.", &b_stack);

	a_stack.nw_layer = network_layer_create(&a_stack);
	a_stack.nw_layer->remote_stack = &b_stack;
	a_stack.nw_layer->transmit_callback = osi_nw_layer_onDiagramTick;
	a_stack.nw_layer->loss_prob = loss_prob;
	a_stack.nw_layer->corrupt_prob = corrupt_prob;
	a_stack.nw_layer->nw_delay = nw_delay;
	a_stack.nw_layer->nw_stddev = nw_stddev;

	b_stack.nw_layer = network_layer_create(&b_stack);
	b_stack.nw_layer->remote_stack = &a_stack;
	b_stack.nw_layer->transmit_callback = osi_nw_layer_onDiagramTick;
	b_stack.nw_layer->loss_prob = loss_prob;
	b_stack.nw_layer->corrupt_prob = corrupt_prob;
	b_stack.nw_layer->nw_delay = nw_delay;
	b_stack.nw_layer->nw_stddev = nw_stddev;

	a_stack.tp_layer = transport_layer_create(&a_stack);
	b_stack.tp_layer = transport_layer_create(&b_stack);
	
	a_stack.app_layer = application_layer_create(&a_stack);
	b_stack.app_layer = application_layer_create(&b_stack);

	DIAG_PRINTF(DIAG_VERBOSE, &a_stack, LAYER_NONE, "Initializing OSI stack.");
	osi_stack_init(&a_stack);
	DIAG_PRINTF(DIAG_VERBOSE, &b_stack, LAYER_NONE, "Initializing OSI stack.");
	osi_stack_init(&b_stack);

	osi_app_layer_onDataGenerateTick(&a_stack);
	if (bidirectional)
		osi_app_layer_onDataGenerateTick(&b_stack);

	while (shouldContinue())
	{
		timer_tickall();
	}

	osi_stack_teardown(&a_stack);

	application_layer_destroy(a_stack.app_layer);
	application_layer_destroy(b_stack.app_layer);

	transport_layer_destroy(a_stack.tp_layer);
	transport_layer_destroy(b_stack.tp_layer);

	network_layer_destroy(a_stack.nw_layer);
	network_layer_destroy(b_stack.nw_layer);

	return 0;
}
