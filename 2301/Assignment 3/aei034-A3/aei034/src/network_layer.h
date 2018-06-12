#pragma once
#ifndef NETWORK_LAYER_H
/// <summary>Header inclusion macro.</summary>
#define NETWORK_LAYER_H 1

#include "osi.h"
#include "transport_package.h"

void osi_nw2tp(osi_stack_t* osi_stack, transport_package_t* tp_pkg);

typedef struct network_layer network_layer_t;

network_layer_t* network_layer_create(osi_stack_t* osi_stack);

void network_layer_destroy(network_layer_t* nw_layer);

void network_layer_init(network_layer_t* nw_layer);

void network_layer_onTeardown(network_layer_t* nw_layer);

void network_layer_onTpSend(network_layer_t* nw_layer, transport_package_t* tp_pkg);

#endif // !NETWORK_LAYER_H
