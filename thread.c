/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 07:59:17 by evvan             #+#    #+#             */
/*   Updated: 2026/06/02 22:13:45 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
	int			first;
	int			second;

	pthread_mutex_lock(&coder->env->state_mutext);
	now = get_time_in_ms();
	if (now < coder->env->dongle_cooldown_ends[coder->left_dongle]
		|| now < coder->env->dongle_cooldown_ends[coder->right_dongle]
		|| !is_prioritarian(coder))
		return (pthread_mutex_unlock(&coder->env->state_mutext), 0);
	pthread_mutex_unlock(&coder->env->state_mutext);
	first = coder->left_dongle;
	second = coder->right_dongle;
	if (first > second)
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	pthread_mutex_lock(&coder->env->dongle_mutext[first]);
	print_status(coder, "has taken a dongle");
	if (coder->env->params->number_of_coder == 1)
		return (0);
	pthread_mutex_lock(&coder->env->dongle_mutext[second]);
	print_status(coder, "has taken a dongle");
	return (1);
}

static void	*handle_single_coder(t_info_coder *coder)
{
	int	dongle;

	dongle = coder->left_dongle;
	pthread_mutex_lock(&coder->env->dongle_mutext[dongle]);
	print_status(coder, "has taken a dongle");
	while (1)
	{
		pthread_mutex_lock(&coder->env->state_mutext);
		if (coder->env->simulation_end)
		{
			pthread_mutex_unlock(&coder->env->state_mutext);
			pthread_mutex_unlock(&coder->env->dongle_mutext[dongle]);
			return (NULL);
		}
		pthread_mutex_unlock(&coder->env->state_mutext);
		usleep(1000);
	}
	return (NULL);
}

void	*coder_routine(void *arg)
{
	t_info_coder	*coder;

	coder = (t_info_coder *)arg;
	if (coder->env->params->number_of_coder == 1)
		return (handle_single_coder(coder));
	while (1)
	{
		pthread_mutex_lock(&coder->env->state_mutext);
		if (coder->env->simulation_end || (coder->env->params
				->number_of_compiles_required > 0 && coder->compile_count
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



if ((dead[0] > dead[1] || (dead[0] == dead[1] && coder->nb_of_coder < coder->env->coder[idx[0]].nb_of_coder))
    || (dead[0] > dead[2] || (dead[0] == dead[2] && coder->nb_of_coder < coder->env->coder[idx[1]].nb_of_coder)))
    return (0);