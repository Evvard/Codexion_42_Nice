/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion_actions.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eolivier <eolivier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 12:00:00 by eolivier          #+#    #+#             */
/*   Updated: 2026/06/01 12:00:00 by eolivier         ###   ########.fr       */
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

static int	neighbor_in_order(int *order, int id, int left_nb, int right_nb)
{
	int	i;

	i = 0;
	while (order[i] != id)
	{
		if (order[i] == left_nb || order[i] == right_nb)
			return (0);
		i++;
	}
	return (1);
}

int	is_prioritarian(t_info_coder *coder)
{
	int	*order;
	int	n;
	int	left_nb;
	int	right_nb;
	int	result;

	n = coder->env->params->number_of_coder;
	left_nb = (coder->nb_of_coder - 2 + n) % n + 1;
	right_nb = (coder->nb_of_coder % n) + 1;
	order = malloc(sizeof(int) * n);
	if (!order)
		return (1);
	minimal_heap_sort(coder->env->coder, n,
		coder->env->params->scheduler, order);
	result = neighbor_in_order(order, coder->nb_of_coder, left_nb, right_nb);
	free(order);
	return (result);
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
