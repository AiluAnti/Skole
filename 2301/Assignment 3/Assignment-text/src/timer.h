#pragma once
#ifndef TIMER_H
/// <summary>Header inclusion macro.</summary>
#define TIMER_H 1

typedef struct tick_timer
{
	int tick_count;
	int target_ticks;
	void(*callback)(void*);
	void* ctx;
} tick_timer_t;

void timer_set(tick_timer_t* timer, int ticks, void(*callback)(void*), void* ctx);

int timer_tickall(void);

#endif // !TIMER_H
