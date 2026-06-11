/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:30:24 by evvan             #+#    #+#             */
/*   Updated: 2026/05/25 20:12:02 by evvan            ###   ########.fr       */
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
	env->dongle_mutex = malloc(sizeof(pthread_mutex_t) * n);
	env->dongle_held = malloc(sizeof(int) * n);
	env->dongle_cooldown_ends = malloc(sizeof(long long) * n);
	env->queue = malloc(sizeof(t_heap));
	if (!env->coder || !env->dongle_mutex || !env->dongle_held
		|| !env->dongle_cooldown_ends || !env->queue)
		return (0);
	env->queue->array = malloc(sizeof(t_request) * n);
	if (!env->queue->array)
		return (0);
	return (1);
}

static void	init_coder(t_environnement *env, int i)
{
	env->coder[i].nb_of_coder = i + 1;
	env->coder[i].compile_count = 0;
	env->coder[i].last_compile_start = env->start_time;
	env->coder[i].waiting = 0;
	env->coder[i].env = env;
	env->coder[i].left_dongle = i;
	env->coder[i].right_dongle = (i + 1) % env->params->number_of_coder;
}

static void	init_env_values(t_environnement *env)
{
	int	i;

	pthread_mutex_init(&env->log_lock, NULL);
	pthread_mutex_init(&env->sched_lock, NULL);
	pthread_cond_init(&env->queue_cond, NULL);
	env->simulation_end = 0;
	env->seq_counter = 0;
	env->start_time = get_time_in_ms();
	env->queue->size = 0;
	env->queue->capacity = env->params->number_of_coder;
	if (strcmp(env->params->scheduler, "fifo") == 0)
		env->queue->cmp = cmp_fifo;
	else
		env->queue->cmp = cmp_edf;
	i = 0;
	while (i < env->params->number_of_coder)
	{
		pthread_mutex_init(&env->dongle_mutex[i], NULL);
		env->dongle_cooldown_ends[i] = 0;
		env->dongle_held[i] = 0;
		init_coder(env, i);
		i++;
	}
}

int	main(int ac, char *av[])
{
	pthread_t		*coders;
	pthread_t		monitor;
	t_environnement	env;

	if (ac != 9)
		return (printf("Usage: %s [coders] [burn] [comp] [debug] [refac] "
				"[compiles] [cooldown] [fifo|edf]\n", av[0]), 1);
	env.params = parser(av);
	if (!env.params)
		return (printf("Error: invalid arguments.\n"), 1);
	if (!init_allocations(&env))
		return (printf("Error: allocation failure.\n"), 1);
	init_env_values(&env);
	coders = malloc(sizeof(pthread_t) * env.params->number_of_coder);
	if (coders)
	{
		run_threads(&env, &monitor, coders);
		free(coders);
	}
	free_all(&env);
	return (0);
}
