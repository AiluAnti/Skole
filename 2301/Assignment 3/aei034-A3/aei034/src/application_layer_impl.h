/// @file application_layer_impl.h
/// <summary>Contains the implementation details of the application layer in the OSI-stack.</summary>
/// <remarks>This file contains declarations used by the network simulator. Students are not required to understand the details of the declarations within this file.</remarks>
/// <seealso cref="application_layer.h"/>

#pragma once
#ifndef APPLICATION_LAYER_IMPL_H
/// <summary>Header inclusion macro.</summary>
#define APPLICATION_LAYER_IMPL_H 1

#include "timer.h"
#include "application_layer.h"

/// <summary>Defines the maximum number of bytes that can be used as one application data transmission buffer.</summary>
/// <value>This value is defined to be a constant unsigned integer value.</value>
#define MAX_APP_DATA_SIZE 64U

/// <summary>Type representing a data node containing application layer data in the application layer data queue.</summary>
typedef struct application_data_node application_data_node_t;

/// <summary>Implements the <see cref="application_data_node_t"/> data type.</summary>
struct application_data_node
{
	/// <summary>The number of valid bytes of data that is stored in the <see cref="data"/> member of this node.</summary>
	/// <value>The value of the <see cref="size"/> member must not exceed <see cref="MAX_APP_DATA_SIZE"/>.</value>
	/// <remarks>If the value of this member is <c>0U</c> (zero) the node does not contain any meaningful data.</remarks>
	size_t size;
	/// <summary>The index into the <see cref="data"/> member that indicates the first next byte of data that can be consumed in this node.</summary>
	/// <value>The value of the <see cref="idx"/> member must not exceed the value of the <see cref="size"/> member.</value>
	size_t idx;
	/// <summary>A buffer of static size <see cref="MAX_APP_DATA_SIZE"/> that contains the data stored in this node.</summary>
	/// <value>Only values with indicies from <see cref="idx"/> up to but excluding <see cref="size"/> can be meaningfully accessed within this buffer.</value>
	char data[MAX_APP_DATA_SIZE];
	/// <summary>A pointer to the data node that follows after this node or <c><see cref="NULL"/></c> if this is the final node.</summary>
	application_data_node_t* next;
};

/// <summary>Implements the <see cref="application_layer_t"/> datatype.</summary>
struct application_layer
{
	/// <summary>A pointer to the OSI-stack which this application layer is part of.</summary>
	/// <value>The value of this member must not be <c><see cref="NULL"/></c>.</value>
	osi_stack_t* osi_stack;
	/// <summary>Indicates how many times this application layer has generated new data.</summary>
	/// <remarks>This value does not indicate how many bytes actually were generated.</remarks>
	int dataGenerateCount;
	/// <summary>The timer that belongs to this application layer.</summary>
	/// <remarks>When this timer times out, the application layer will generate new data and attempt to transmit it by sending it downwards in the OSI-stack.</remarks>
	tick_timer_t dataGenerateTimer;

	/// <summary>Indicates the current start of the data node queue for this application layer.</summary>
	/// <value>The value of this member will be a pointer to a application data node or <c><see cref="NULL"/></c> if the queue is empty.</value>
	/// <remarks>The application layer data node queue stores the nodes of data that are expected to arrive from the underlying transport layer.</remarks>
	application_data_node_t* dataFirst;
	/// <summary>Indicates the current tail of the data node queue for this application layer.</summary>
	/// <value>The value of this member will be a pointer to a application data node or <c><see cref="NULL"/></c> if the queue is empty.</value>
	/// <remarks>The application layer data node queue stores the nodes of data that are expected to arrive from the underlying transport layer.</remarks>
	application_data_node_t* dataLast;
};

#endif // !APPLICATION_LAYER_IMPL_H
