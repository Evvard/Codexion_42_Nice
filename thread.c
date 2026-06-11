/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 07:59:17 by evvan             #+#    #+#             */
/*   Updated: 2026/06/08 21:11:48 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	execute_compile(t_info_coder *c)
{
	t_environnement	*e;
	long long		now;

	e = c->env;
	print_status(c, "is compiling");
	usleep(e->params->time_to_compile * 1000);
	pthread_mutex_lock(&e->sched_lock);
	c->compile_count++;
	now = get_time_in_ms();
	e->dongle_cooldown_ends[c->left_dongle] = now + e->params->dongle_cooldown;
	e->dongle_cooldown_ends[c->right_dongle] = now + e->params->dongle_cooldown;
	e->dongle_held[c->left_dongle] = 0;
	e->dongle_held[c->right_dongle] = 0;
	pthread_cond_broadcast(&e->queue_cond);
	pthread_mutex_unlock(&e->sched_lock);
	pthread_mutex_unlock(&e->dongle_mutex[c->right_dongle]);
	pthread_mutex_unlock(&e->dongle_mutex[c->left_dongle]);
}

static void	take_physical_dongles(t_info_coder *c)
{
	int	first;
	int	second;

	first = c->left_dongle;
	second = c->right_dongle;
	if (first > second)
	{
		first = c->right_dongle;
		second = c->left_dongle;
	}
	pthread_mutex_lock(&c->env->dongle_mutex[first]);
	print_status(c, "has taken a dongle");
	pthread_mutex_lock(&c->env->dongle_mutex[second]);
	print_status(c, "has taken a dongle");
}

static void	*handle_single_coder(t_info_coder *c)
{
	pthread_mutex_lock(&c->env->dongle_mutex[c->left_dongle]);
	print_status(c, "has taken a dongle");
	while (!sim_ended(c->env))
		usleep(1000);
	pthread_mutex_unlock(&c->env->dongle_mutex[c->left_dongle]);
	return (NULL);
}

void	*coder_routine(void *arg)
{
	t_info_coder	*c;

	c = (t_info_coder *)arg;
	if (c->env->params->number_of_coder == 1)
		return (handle_single_coder(c));
	while (!sim_ended(c->env))
	{
		request_dongles(c);
		if (sim_ended(c->env))
			break ;
		take_physical_dongles(c);
		execute_compile(c);
		print_status(c, "is debugging");
		usleep(c->env->params->time_to_debug * 1000);
		print_status(c, "is refactoring");
		usleep(c->env->params->time_to_refactor * 1000);
	}
	return (NULL);
}

int	run_threads(t_environnement *env, pthread_t *mon, pthread_t *co)
{
	int	i;

	if (pthread_create(mon, NULL, &monitor_routine, env) != 0)
		return (0);
	i = 0;
	while (i < env->params->number_of_coder)
	{
		if (pthread_create(&co[i], NULL, &coder_routine, &env->coder[i]) != 0)
			return (0);
		i++;
	}
	i = 0;
	while (i < env->params->number_of_coder)
		pthread_join(co[i++], NULL);
	pthread_join(*mon, NULL);
	return (1);
}
