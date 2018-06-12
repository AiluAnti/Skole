/// @file transport_package_impl.h
/// <summary>Contains the implementation details of transport packages.</summary>
/// <remarks>Students are required to redefine the implementation details of the transport package structure.</remarks>

#pragma once
#ifndef TRANSPORT_PACKAGE_IMPL_H
/// <summary>Header inclusion macro.</summary>
#define TRANSPORT_PACKAGE_IMPL_H 1
#include <stdlib.h>
#include <stdio.h>
#include "transport_package.h"

/// <summary>Implements the <see cref="transport_package_t"/> datatype.</summary>
/// <remarks>Students should modify/add fields declared in this structure according to their own understanding of transport packages.</remarks>
struct transport_package
{
	int ack;
	int checksum;
	size_t size;
	void* data;
};

#endif // !TRANSPORT_PACKAGE_IMPL_H