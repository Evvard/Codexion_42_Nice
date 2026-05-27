/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eolivier <eolivier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:30:24 by evvan             #+#    #+#             */
/*   Updated: 2026/05/27 16:35:20 by eolivier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	get_time_in_ms(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL) == -1)
		return (0);
	return ((tv.tv_sec * 1000LL) + (tv.tv_usec / 1000LL));
}

static int	init_allocations(t_environnement *env)
{
	int	n;

	n = env->params->number_of_coder;
	env->coder = malloc(sizeof(t_info_coder) * n);
	env->dongle_mutext = malloc(sizeof(pthread_mutex_t) * n);
	env->dongle_cooldown_ends = malloc(sizeof(long long) * n);
	if (!env->coder || !env->dongle_mutext || !env->dongle_cooldown_ends)
		return (0);
	return (1);
}

static void	init_env_values(t_environnement *env)
{
	int	i;

	pthread_mutex_init(&env->log_mutext, NULL);
	pthread_mutex_init(&env->state_mutext, NULL);
	env->simulation_end = 0;
	env->start_time = get_time_in_ms();
	i = 0;
	while (i < env->params->number_of_coder)
	{
		pthread_mutex_init(&env->dongle_mutext[i], NULL);
		env->dongle_cooldown_ends[i] = 0;
		env->coder[i].nb_of_coder = i + 1;
		env->coder[i].compile_count = 0;
		env->coder[i].last_compile_start = env->start_time;
		env->coder[i].env = env;
		env->coder[i].left_dongle = i;
		env->coder[i].right_dongle = (i + 1) % env->params->number_of_coder;
		i++;
	}
}

static int	run_threads(t_environnement *env, pthread_t *th_mon,
		pthread_t *th_c)
{
	int	i;

	if (pthread_create(th_mon, NULL, &monitor_routine, env) != 0)
		return (0);
	i = 0;
	while (i < env->params->number_of_coder)
	{
		if (pthread_create(&th_c[i], NULL, &coder_routine,
				&env->coder[i]) != 0)
			return (0);
		i++;
	}
	i = 0;
	while (i < env->params->number_of_coder)
	{
		pthread_join(th_c[i], NULL);
		i++;
	}
	pthread_join(*th_mon, NULL);
	return (1);
}

int	main(int ac, char *av[])
{
	pthread_t		*threads_coders;
	pthread_t		thread_monitor;
	t_parsing_list	*argument;
	t_environnement	env;

	if (ac != 9)
	{
		printf("Usage: %s [coder] [burn] [comp] [deb]...[sched]\n", av[0]);
		return (1);
	}
	argument = parser(av);
	if (!argument)
		return (printf("Arguments invalides.\n"), 1);
	env.params = argument;
	if (!init_allocations(&env))
		return (free(argument->scheduler), free(argument), 1);
	init_env_values(&env);
	threads_coders = malloc(sizeof(pthread_t) * env.params->number_of_coder);
	if (threads_coders)
	{
		run_threads(&env, &thread_monitor, threads_coders);
		free(threads_coders);
	}
	free_all(&env);
	return (0);
}
