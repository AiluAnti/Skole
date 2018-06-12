#pragma once
#ifndef TRANSPORT_PACKAGE_H
/// <summary>Header inclusion macro.</summary>
#define TRANSPORT_PACKAGE_H 1

#include <stdint.h>
#include <stdlib.h>

typedef struct transport_package transport_package_t;

transport_package_t* transport_pkg_create(void* data, size_t size, uint8_t ack, uint8_t seq);

void transport_pkg_destroy(transport_package_t* tp_pkg);

transport_package_t* transport_pkg_copy(transport_package_t* src);

size_t transport_pkg_size(transport_package_t* pkg);

void* transport_pkg_data(transport_package_t* pkg);

#endif // !TRANSPORT_PACKAGE_H
