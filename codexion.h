/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:30:22 by evvan             #+#    #+#             */
/*   Updated: 2026/06/08 21:12:04 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <time.h>
# include <sys/time.h>
# include <string.h>

typedef struct s_request
{
	int			coder_id;
	long long	deadline;
	long long	seq;
}				t_request;

typedef struct s_heap
{
	t_request	*array;
	int			size;
	int			capacity;
	int			(*cmp)(t_request *a, t_request *b);
}				t_heap;

typedef struct s_parsing_list
{
	int		number_of_coder;
	int		time_to_burnout;
	int		time_to_compile;
	int		time_to_debug;
	int		time_to_refactor;
	int		number_of_compiles_required;
	int		dongle_cooldown;
	char	*scheduler;
}			t_parsing_list;

typedef struct s_coder
{
	int				nb_of_coder;
	long long		last_compile_start;
	int				compile_count;
	int				left_dongle;
	int				right_dongle;
	int				waiting;
	t_request		req;
	struct s_env	*env;
}					t_info_coder;

typedef struct s_env
{
	t_parsing_list	*params;
	t_info_coder	*coder;
	pthread_mutex_t	*dongle_mutex;
	int				*dongle_held;
	long long		*dongle_cooldown_ends;
	t_heap			*queue;
	pthread_mutex_t	sched_lock;
	pthread_cond_t	queue_cond;
	pthread_mutex_t	log_lock;
	long long		seq_counter;
	int				simulation_end;
	long long		start_time;
}					t_environnement;

t_parsing_list	*parser(char **things);
long long		get_time_in_ms(void);
void			print_status(t_info_coder *coder, char *status);
void			*monitor_routine(void *arg);
void			*coder_routine(void *arg);
int				run_threads(t_environnement *e, pthread_t *m, pthread_t *c);
void			execute_compile(t_info_coder *coder);
void			request_dongles(t_info_coder *coder);
void			free_all(t_environnement *env);
int				sim_ended(t_environnement *env);
int				cmp_fifo(t_request *a, t_request *b);
int				cmp_edf(t_request *a, t_request *b);
void			heap_push(t_heap *heap, t_request req);
void			heap_remove(t_heap *heap, int coder_id);
int				heap_peek_id(t_heap *heap);

#endif
