/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eolivier <eolivier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 20:09:25 by evvan             #+#    #+#             */
/*   Updated: 2026/06/01 11:46:28 by eolivier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	print_status(t_info_coder *coder, char *status)
{
	long long	timestamp;

	pthread_mutex_lock(&coder->env->log_mutext);
	pthread_mutex_lock(&coder->env->state_mutext);
	if (!coder->env->simulation_end)
	{
		timestamp = get_time_in_ms() - coder->env->start_time;
		printf("%lld %d %s\n", timestamp, coder->nb_of_coder, status);
	}
	pthread_mutex_unlock(&coder->env->state_mutext);
	pthread_mutex_unlock(&coder->env->log_mutext);
}

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
	if (left < heap->size && heap->data[left].priority < heap->data[smallest].priority)
		smallest = left;
	if (right < heap->size && heap->data[right].priority < heap->data[smallest].priority)
		smallest = right;
	if (smallest != idx)
	{
		swap_nodes(&heap->data[idx], &heap->data[smallest]);
		heapify_down(heap, smallest);
	}
}

void	minimal_heap_sort(t_info_coder *coders, int n, char *sched, int *rs)
{
	t_heap_node	*nodes = malloc(sizeof(t_heap_node) * n);
	t_heap		heap;
	int			i;

	if (!nodes)
		return ;
	i = -1;
	while (++i < n)
	{
		nodes[i].coder_id = coders[i].nb_of_coder;
		if (strcmp(sched, "edf") == 0)
			nodes[i].priority = coders[i].last_compile_start + coders[i].env->params->time_to_burnout;
		else
			nodes[i].priority = coders[i].last_compile_start;
	}
	heap.data = nodes;
	heap.size = n;
	for (i = (n / 2) - 1; i >= 0; i--)
		heapify_down(&heap, i);
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

int	is_prioritarian(t_info_coder *coder)
{
	int	*order;
	int	i;
	int	is_first;

	order = malloc(sizeof(int) * coder->env->params->number_of_coder);
	if (!order)
		return (1);
	minimal_heap_sort(coder->env->coder, coder->env->params->number_of_coder,
		coder->env->params->scheduler, order);
	is_first = 1;
	i = 0;
	while (order[i] != coder->nb_of_coder)
	{
		if (order[i] == (coder->nb_of_coder - 2 + coder->env->params->number_of_coder) % coder->env->params->number_of_coder + 1
			|| order[i] == (coder->nb_of_coder % coder->env->params->number_of_coder) + 1)
		{
			is_first = 0;
			break ;
		}
		i++;
	}
	free(order);
	return (is_first);
}

void	execute_compile(t_info_coder *coder)
{
	pthread_mutex_lock(&coder->env->state_mutext);
	coder->last_compile_start = get_time_in_ms();
	pthread_mutex_unlock(&coder->env->state_mutext);
	print_status(coder, "is compiling");
	usleep(coder->env->params->time_to_compile * 1000);
	pthread_mutex_lock(&coder->env->state_mutext);
	coder->env->dongle_cooldown_ends[coder->left_dongle] = get_time_in_ms()
		+ coder->env->params->dongle_cooldown;
	coder->env->dongle_cooldown_ends[coder->right_dongle] = get_time_in_ms()
		+ coder->env->params->dongle_cooldown;
	coder->compile_count++;
	pthread_mutex_unlock(&coder->env->state_mutext);
	pthread_mutex_unlock(&coder->env->dongle_mutext[coder->right_dongle]);
	pthread_mutex_unlock(&coder->env->dongle_mutext[coder->left_dongle]);
}

void	free_all(t_environnement *env)
{
	int	i;

	i = -1;
	while (++i < env->params->number_of_coder)
		pthread_mutex_destroy(&env->dongle_mutext[i]);
	pthread_mutex_destroy(&env->log_mutext);
	pthread_mutex_destroy(&env->state_mutext);
	free(env->coder);
	free(env->dongle_mutext);
	free(env->dongle_cooldown_ends);
	free(env->params->scheduler);
	free(env->params);
}
