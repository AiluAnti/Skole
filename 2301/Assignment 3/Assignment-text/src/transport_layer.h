#pragma once
#ifndef TRANSPORT_LAYER_H
/// <summary>Header inclusion macro.</summary>
#define TRANSPORT_LAYER_H 1

#include <stdlib.h>

#include "timer.h"
#include "osi.h"
#include "transport_package.h"

void osi_tp2app(osi_stack_t* osi_stack, void* data, size_t size);

void osi_tp2nw(osi_stack_t* osi_stack, transport_package_t* tp_pkg);

typedef struct transport_layer transport_layer_t;

transport_layer_t* transport_layer_create(osi_stack_t* osi_stack);

void transport_layer_init(transport_layer_t* tp_layer);

void transport_layer_destroy(transport_layer_t* tp_layer);

void transport_layer_onAppSend(transport_layer_t* tp_layer, void* data, size_t size);

void transport_layer_onNwReceive(transport_layer_t* tp_layer, transport_package_t* tp_pkg);

void transport_layer_onLayerTimeout(transport_layer_t* tp_layer);

void transport_layer_onTeardown(transport_layer_t* tp_layer);

#endif // !TRANSPORT_LAYER_H
