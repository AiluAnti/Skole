/// @file application_layer.h
/// <summary>Contains the externally visible API of the application layer in the OSI-stack.</summary>
/// <remarks>This file contains declarations used by the network simulator. Students are not required to understand the details of the declarations within this file.</remarks>
/// <seealso cref="application_layer_impl.h"/>

#pragma once
#ifndef APPLICATION_LAYER_H
/// <summary>Header inclusion macro.</summary>
#define APPLICATION_LAYER_H 1

#include <stdlib.h>

#include "osi.h"

/// <summary>Type representing the implementation of the application layer in an OSI-stack.</summary>
typedef struct application_layer application_layer_t;

/// <summary>Pushes application layer data down to the transport layer in the OSI-stack.</summary>
/// <param name="osi_stack">A pointer to the OSI stack the calling application layer belongs to.</param>
/// <param name="data">A pointer to a memory buffer containing the data to transmit. It must be at least <paramref name="size"/> bytes long.</param>
/// <param name="size">The number of valid bytes contained in the buffer pointed to by <paramref name="data"/>.</param>
/// <remarks>This function is implemented by the precode. Students are neither required to implement nor to understand the details of this function.</remarks>
void osi_app2tp(osi_stack_t* osi_stack, void* data, size_t size);

/// <summary>Creates a new application layer instance for the specified OSI-stack.</summary>
/// <param name="osi_stack">A pointer to the OSI-stack for which the layer will be created.</param>
/// <returns>A pointer to the created application layer. It must be deallocated with a call to <see cref="application_layer_destroy"/> when it is no longer needed.</returns>
/// <remarks>
/// <see cref="malloc"/> is used to allocate memory on the system.<br/>
/// This function is implemented by the precode. Students are neither required to implement nor to understand the details of this function.
/// </remarks>
/// <exception cref="NULL"><paramref name="osi_stack"/> is <c><see cref="NULL"/></c>.</exception>
/// <exception cref="NULL">There is not enough memory available on the system to allocate a new application layer. <c><see cref="errno"/></c> is set to an appropiate error value.</exception>
application_layer_t* application_layer_create(osi_stack_t* osi_stack);

/// <summary>Frees the memory occupied by the specified application layer.</summary>
/// <param name="app_layer">A pointer to the application layer to destroy.</param>
/// <remarks>
/// If <paramref name="app_layer"/> is <c><see cref="NULL"/></c>, this function has no effect.<br/>
/// This function is implemented by the precode. Students are neither required to implement nor to understand the details of this function.
/// </remarks>
void application_layer_destroy(application_layer_t* app_layer);

/// <summary>Initializes the specified application layer code after all components in the OSI stack have been created.</summary>
/// <param name="app_layer">A pointer to the application layer to initialize.</param>
/// <remarks>
/// If <paramref name="app_layer"/> is <c><see cref="NULL"/></c>, this function has no effect.<br/>
/// This function is implemented by the precode. Students are neither required to implement nor to understand the details of this function.
/// </remarks>
void application_layer_init(application_layer_t* app_layer);

/// <summary>Called when the application layer receives data from the underlying transport layer.</summary>
/// <param name="app_layer">A pointer to the application layer that receives the data.</param>
/// <param name="data">A pointer to the buffer that contains the data. It must be at least <paramref name="size"/> bytes long.</param>
/// <param name="size">The number of valid bytes contained in the buffer pointed to by <paramref name="data"/>.</param>
/// <remarks>
/// If either <paramref name="app_layer"/> or <paramref name="data"/> is <c><see cref="NULL"/></c>, this function has no effect.<br/>
/// If <paramref name="size"/> is <c>0</c> (zero), it signals the application layer that the last byte has been received and that no more communication is possible. This will cause the application layer to tear down the OSI-stack.<br/>
/// This function is implemented by the precode. Students are neither required to implement nor to understand the details of this function.
/// </remarks>
void application_layer_onTpReceive(application_layer_t* app_layer, void* data, size_t size);

/// <summary>Called when the application layer is being torn down as part of the OSI-stack being torn down. All underlying layers will still be intact when this function is called.</summary>
/// <param name="app_layer">A pointer to the application layer being torn down.</param>
/// <remarks>
/// If <paramref name="app_layer"/> is <c><see cref="NULL"/></c>, this function has no effect.<br/>
/// This function is implemented by the precode. Students are neither required to implement nor to understand the details of this function.
/// </remarks>
void application_layer_onTeardown(application_layer_t* app_layer);

#endif // !APPLICATION_LAYER_H
