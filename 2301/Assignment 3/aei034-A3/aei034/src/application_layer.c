#include <stdlib.h>

#include "diagnostics.h"
#include "application_layer_impl.h"

application_layer_t* application_layer_create(osi_stack_t* osi_stack)
{
	application_layer_t* app_layer;

	if (!osi_stack)
		return NULL;

	app_layer = (application_layer_t*)malloc(sizeof(application_layer_t));
	if (!app_layer)
		return NULL;

	DIAG_PRINTF(DIAG_DEBUG, osi_stack, LAYER_APP, "Creating application layer %#p", app_layer);
	app_layer->osi_stack = osi_stack;

	return app_layer;
}

void application_layer_destroy(application_layer_t* app_layer)
{
	if (app_layer)
	{
		DIAG_PRINTF(DIAG_DEBUG, app_layer->osi_stack, LAYER_APP, "Freeing application layer %#p", app_layer);
		free(app_layer);
	}
}

void application_layer_init(application_layer_t* app_layer)
{
	if (!app_layer)
		return;

	DIAG_PRINTF(DIAG_DEBUG, app_layer->osi_stack, LAYER_APP, "Initializing application layer %#p", app_layer);
	app_layer->dataGenerateCount = 0;
	app_layer->dataLast = NULL;
	app_layer->dataFirst = NULL;
}

void application_layer_onTpReceive(application_layer_t* app_layer, void* data, size_t size)
{
	size_t i;
	char* buffer;
	application_data_node_t* curr;

	if (!app_layer || !data)
		return;
	else if (!size)
	{
		DIAG_PRINTF(DIAG_WARNING, app_layer->osi_stack, LAYER_APP, "Received data package of length 0. Shutting down connection.");
		osi_stack_teardown(app_layer->osi_stack);
		return;
	}

	buffer = (char*)data;
	if (!app_layer->dataFirst)
	{
		DIAG_PRINTF(DIAG_CRITICAL, app_layer->osi_stack, LAYER_APP, "Received data on application layer without expecting any data.");
		DIAG_PRINTF(DIAG_CRITICAL, app_layer->osi_stack, LAYER_APP, "Program terminating!");
		exit(EXIT_FAILURE);
	}

	curr = app_layer->dataFirst;
	app_layer->dataFirst = curr->next;
	if (!(app_layer->dataFirst))
		app_layer->dataLast = NULL;

	for (i = 0U; i < size; i++, curr->idx += 1)
	{
	loopBegin:
		if (!curr || !(curr->data) || !(curr->size))
		{
			DIAG_PRINTF(DIAG_CRITICAL, app_layer->osi_stack, LAYER_APP, "Received data on application layer without expecting any data.");
			DIAG_PRINTF(DIAG_CRITICAL, app_layer->osi_stack, LAYER_APP, "Program terminating!");
			exit(EXIT_FAILURE);
		}
		else if (curr->idx >= curr->size)
		{
			free(curr);
			curr = app_layer->dataFirst;
			app_layer->dataFirst = curr ? curr->next : NULL;
			if (!(app_layer->dataFirst))
				app_layer->dataLast = NULL;
			goto loopBegin;
		}

		if (buffer[i] != curr->data[curr->idx])
		{
			DIAG_PRINTF(DIAG_CRITICAL, app_layer->osi_stack, LAYER_APP, "Received data does not match expected data. Idx = %Iu, Actual = %#04X, Expected = %#04X", i, (unsigned int)buffer[i], (unsigned int)curr->data[curr->idx]);
			DIAG_PRINTF(DIAG_CRITICAL, app_layer->osi_stack, LAYER_APP, "Program terminating!");
			exit(EXIT_FAILURE);
		}
	}

	if (curr && curr->idx < curr->size)
	{
		app_layer->dataFirst = curr;
		if (!(app_layer->dataLast))
			app_layer->dataLast = curr;
	}
}

void application_layer_onTeardown(application_layer_t* app_layer)
{
	if (!app_layer)
		return;

	DIAG_PRINTF(DIAG_INFO, app_layer->osi_stack, LAYER_APP, "Layer being torn down.");
	//app_layer->dataGenerateCount = 0;
	app_layer->dataGenerateTimer.callback = NULL;
	app_layer->dataGenerateTimer.ctx = NULL;
	app_layer->dataGenerateTimer.target_ticks = 0;
	app_layer->dataGenerateTimer.tick_count = 0;
}
