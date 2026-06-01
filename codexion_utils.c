/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eolivier <eolivier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 20:09:25 by evvan             #+#    #+#             */
/*   Updated: 2026/06/01 12:00:00 by eolivier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap_nodes(t_heap_node *a, t_heap_node *b)
{
	t_heap_node	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static void	heapify_down(t_heap *heap, int idx)
{
	int	smallest;
	int	left;
	int	right;

	smallest = idx;
	left = 2 * idx + 1;
	right = 2 * idx + 2;
	if (left < heap->size
		&& heap->data[left].priority < heap->data[smallest].priority)
		smallest = left;
	if (right < heap->size
		&& heap->data[right].priority < heap->data[smallest].priority)
		smallest = right;
	if (smallest != idx)
	{
		swap_nodes(&heap->data[idx], &heap->data[smallest]);
		heapify_down(heap, smallest);
	}
}

static void	fill_heap_nodes(t_heap_node *nodes, t_info_coder *coders,
				int n, char *sched)
{
	int	i;

	i = -1;
	while (++i < n)
	{
		nodes[i].coder_id = coders[i].nb_of_coder;
		if (strcmp(sched, "edf") == 0)
			nodes[i].priority = coders[i].last_compile_start
				+ coders[i].env->params->time_to_burnout;
		else
			nodes[i].priority = coders[i].last_compile_start;
	}
}

void	get_ordered_dongles(t_info_coder *coder, int *first, int *second)
{
	if (coder->left_dongle < coder->right_dongle)
	{
		*first = coder->left_dongle;
		*second = coder->right_dongle;
	}
	else
	{
		*first = coder->right_dongle;
		*second = coder->left_dongle;
	}
}

void	minimal_heap_sort(t_info_coder *coders, int n, char *sched, int *rs)
{
	t_heap_node	*nodes;
	t_heap		heap;
	int			i;

	nodes = malloc(sizeof(t_heap_node) * n);
	if (!nodes)
		return ;
	fill_heap_nodes(nodes, coders, n, sched);
	heap.data = nodes;
	heap.size = n;
	i = (n / 2) - 1;
	while (i >= 0)
		heapify_down(&heap, i--);
	i = 0;
	while (heap.size > 0)
	{
		rs[i++] = heap.data[0].coder_id;
		swap_nodes(&heap.data[0], &heap.data[heap.size - 1]);
		heap.size--;
		heapify_down(&heap, 0);
	}
	free(nodes);
}
