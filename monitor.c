/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 21:30:00 by evvan             #+#    #+#             */
/*   Updated: 2026/06/08 21:30:00 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	sim_ended(t_environnement *env)
{
	int	value;

	pthread_mutex_lock(&env->sched_lock);
	value = env->simulation_end;
	pthread_mutex_unlock(&env->sched_lock);
	return (value);
}

static int	check_burnout(t_environnement *env, int i)
{
	long long	now;

	now = get_time_in_ms();
	if (now - env->coder[i].last_compile_start > env->params->time_to_burnout)
	{
		env->simulation_end = 1;
		pthread_mutex_lock(&env->log_lock);
		printf("%lld %d burned out\n", now - env->start_time, i + 1);
		pthread_mutex_unlock(&env->log_lock);
		return (1);
	}
	return (0);
}

static int	all_reached_quota(t_environnement *env)
{
	int	i;

	if (env->params->number_of_compiles_required <= 0)
		return (0);
	i = 0;
	while (i < env->params->number_of_coder)
	{
		if (env->coder[i].compile_count
			< env->params->number_of_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

static int	monitor_check(t_environnement *env)
{
	int	i;

	i = 0;
	while (i < env->params->number_of_coder)
	{
		if (check_burnout(env, i))
			return (1);
		i++;
	}
	if (all_reached_quota(env))
	{
		env->simulation_end = 1;
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_environnement	*env;

	env = (t_environnement *)arg;
	while (1)
	{
		pthread_mutex_lock(&env->sched_lock);
		if (env->simulation_end || monitor_check(env))
		{
			pthread_cond_broadcast(&env->queue_cond);
			pthread_mutex_unlock(&env->sched_lock);
			return (NULL);
		}
		pthread_mutex_unlock(&env->sched_lock);
		usleep(500);
	}
}
