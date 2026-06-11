/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 20:09:25 by evvan             #+#    #+#             */
/*   Updated: 2026/06/08 21:12:07 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	cmp_fifo(t_request *a, t_request *b)
{
	return (a->seq < b->seq);
}

int	cmp_edf(t_request *a, t_request *b)
{
	if (a->deadline != b->deadline)
		return (a->deadline < b->deadline);
	return (a->seq < b->seq);
}

int	heap_peek_id(t_heap *h)
{
	if (h->size == 0)
		return (-1);
	return (h->array[0].coder_id);
}

void	print_status(t_info_coder *coder, char *status)
{
	long long	timestamp;

	pthread_mutex_lock(&coder->env->sched_lock);
	if (!coder->env->simulation_end)
	{
		timestamp = get_time_in_ms() - coder->env->start_time;
		pthread_mutex_lock(&coder->env->log_lock);
		printf("%lld %d %s\n", timestamp, coder->nb_of_coder, status);
		pthread_mutex_unlock(&coder->env->log_lock);
	}
	pthread_mutex_unlock(&coder->env->sched_lock);
}

void	free_all(t_environnement *env)
{
	int	i;

	i = 0;
	while (i < env->params->number_of_coder)
	{
		pthread_mutex_destroy(&env->dongle_mutex[i]);
		i++;
	}
	pthread_mutex_destroy(&env->log_lock);
	pthread_mutex_destroy(&env->sched_lock);
	pthread_cond_destroy(&env->queue_cond);
	free(env->coder);
	free(env->dongle_mutex);
	free(env->dongle_held);
	free(env->dongle_cooldown_ends);
	free(env->queue->array);
	free(env->queue);
	free(env->params);
}
