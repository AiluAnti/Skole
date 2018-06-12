#pragma once
#ifndef NETWORK_LAYER_IMPL_H
/// <summary>Header inclusion macro.</summary>
#define NETWORK_LAYER_IMPL_H 1

#include "network_layer.h"

struct network_layer
{
	osi_stack_t* osi_stack;
	osi_stack_t* remote_stack;

	void(*transmit_callback)(void*[3]);

	float loss_prob, corrupt_prob;
	float nw_delay, nw_stddev;
};

#endif // !NETWORK_LAYER_IMPL_H
