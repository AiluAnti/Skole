#pragma once
#ifndef OSI_H
/// <summary>Header inclusion macro.</summary>
#define OSI_H 1

typedef struct osi_stack osi_stack_t;

void osi_stack_init(osi_stack_t* osi_stack);

void osi_stack_teardown(osi_stack_t* osi_stack);

#endif // !OSI_H
