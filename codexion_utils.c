/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 20:09:25 by evvan             #+#    #+#             */
/*   Updated: 2026/05/25 20:16:16 by evvan            ###   ########.fr       */
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

int	is_prioritarian(t_info_coder *coder)
{
	long long	dead[3];
	int			idx[2];

	if (strcmp(coder->env->params->scheduler, "fifo") == 0)
		return (1);
	idx[0] = (coder->nb_of_coder - 2 + coder->env->params->number_of_coder)
		% coder->env->params->number_of_coder;
	idx[1] = coder->nb_of_coder % coder->env->params->number_of_coder;
	dead[0] = coder->last_compile_start + coder->env->params->time_to_burnout;
	dead[1] = coder->env->coder[idx[0]].last_compile_start
		+ coder->env->params->time_to_burnout;
	dead[2] = coder->env->coder[idx[1]].last_compile_start
		+ coder->env->params->time_to_burnout;
	if (dead[0] > dead[1] || dead[0] > dead[2])
		return (0);
	return (1);
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

int	check_coder_burnout(t_environnement *env, int i)
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
