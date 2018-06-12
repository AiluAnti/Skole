#include <stdlib.h>

#include "timer.h"

typedef struct timer_node timer_node_t;
struct timer_node
{
	tick_timer_t* value;
	timer_node_t* next;
};

static timer_node_t* timer_lst_head = NULL;

void timer_set(tick_timer_t* timer, int ticks, void(*callback)(void*), void* ctx)
{
	timer_node_t* nd;

	if (!timer)
		return;

	timer->tick_count = 0;
	timer->target_ticks = ticks;
	timer->callback = callback;
	timer->ctx = ctx;

	nd = (timer_node_t*)malloc(sizeof(timer_node_t));
	if (!nd)
		return;

	nd->value = timer;
	nd->next = timer_lst_head;
	timer_lst_head = nd;
}

int timer_tickall(void)
{
	int count = 0;
	timer_node_t* prev;
	timer_node_t* curr;
	tick_timer_t* t;

	for (count = 0, prev = NULL, curr = timer_lst_head; curr; prev = curr, curr = curr->next)
	{
		t = curr->value;
		if (!t)
			continue;

		t->tick_count += 1;
		if (t->tick_count < t->target_ticks)
			continue;
		count++;

		if (prev)
			prev->next = curr->next;
		else if (curr == timer_lst_head)
			timer_lst_head = curr->next;
		free(curr);
		curr = prev;

		if (t->callback)
			t->callback(t->ctx);

		if (!curr)
			break;
	}

	return count;
}
