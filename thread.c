/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 07:59:17 by evvan             #+#    #+#             */
/*   Updated: 2026/05/25 20:18:19 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_coder_burnout(t_environnement *env, int i)
{
	long long	now;

	now = get_time_in_ms();
	if (now - env->coder[i].last_compile_start > env->params->time_to_burnout)
	{
		env->simulation_end = 1;
		pthread_mutex_lock(&env->log_mutext);
		printf("%lld %d burned out\n", now - env->start_time,
			env->coder[i].nb_of_coder);
		pthread_mutex_unlock(&env->log_mutext);
		return (1);
	}
	return (0);
}

static int	check_all_coders(t_environnement *env)
{
	int	i;
	int	all_done;

	i = -1;
	all_done = 1;
	while (++i < env->params->number_of_coder)
	{
		if (check_coder_burnout(env, i))
			return (-1);
		if (env->params->number_of_compiles_required <= 0
			|| env->coder[i].compile_count
			< env->params->number_of_compiles_required)
			all_done = 0;
	}
	return (all_done);
}

void	*monitor_routine(void *arg)
{
	t_environnement	*env;
	int				status;

	env = (t_environnement *)arg;
	while (1)
	{
		pthread_mutex_lock(&env->state_mutext);
		if (env->simulation_end)
			return (pthread_mutex_unlock(&env->state_mutext), NULL);
		status = check_all_coders(env);
		if (status == -1)
			return (pthread_mutex_unlock(&env->state_mutext), NULL);
		if (env->params->number_of_compiles_required > 0 && status == 1)
			return (env->simulation_end = 1,
				pthread_mutex_unlock(&env->state_mutext), NULL);
		pthread_mutex_unlock(&env->state_mutext);
		usleep(1000);
	}
}

static int	try_take_dongles(t_info_coder *coder)
{
	long long	now;

	pthread_mutex_lock(&coder->env->state_mutext);
	now = get_time_in_ms();
	if (now < coder->env->dongle_cooldown_ends[coder->left_dongle]
		|| now < coder->env->dongle_cooldown_ends[coder->right_dongle])
		return (pthread_mutex_unlock(&coder->env->state_mutext), 0);
	if (!is_prioritarian(coder))
		return (pthread_mutex_unlock(&coder->env->state_mutext), 0);
	pthread_mutex_unlock(&coder->env->state_mutext);
	pthread_mutex_lock(&coder->env->dongle_mutext[coder->left_dongle]);
	print_status(coder, "has taken a dongle");
	pthread_mutex_lock(&coder->env->dongle_mutext[coder->right_dongle]);
	print_status(coder, "has taken a dongle");
	return (1);
}

void	*coder_routine(void *arg)
{
	t_info_coder	*coder;

	coder = (t_info_coder *)arg;
	while (1)
	{
		pthread_mutex_lock(&coder->env->state_mutext);
		if (coder->env->simulation_end
			|| (coder->env->params->number_of_compiles_required > 0
				&& coder->compile_count
				>= coder->env->params->number_of_compiles_required))
			return (pthread_mutex_unlock(&coder->env->state_mutext), NULL);
		pthread_mutex_unlock(&coder->env->state_mutext);
		if (try_take_dongles(coder))
		{
			execute_compile(coder);
			print_status(coder, "is debugging");
			usleep(coder->env->params->time_to_debug * 1000);
			print_status(coder, "is refactoring");
			usleep(coder->env->params->time_to_refactor * 1000);
		}
		else
			usleep(50);
	}
	return (NULL);
}
