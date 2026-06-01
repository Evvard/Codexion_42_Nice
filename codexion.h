/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eolivier <eolivier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:30:22 by evvan             #+#    #+#             */
/*   Updated: 2026/06/01 11:46:51 by eolivier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <limits.h>
# include <unistd.h>
# include <time.h>
# include <sys/time.h>
# include <string.h>

typedef struct t_coder
{
	int				nb_of_coder;
	long long		last_compile_start;
	int				compile_count;
	int				left_dongle;
	int				right_dongle;
	struct t_env	*env;
}					t_info_coder;

typedef struct t_list
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

typedef struct t_env
{
	t_parsing_list	*params;
	t_info_coder	*coder;
	pthread_mutex_t	*dongle_mutext;
	long long		*dongle_cooldown_ends;
	pthread_mutex_t	log_mutext;
	pthread_mutex_t	state_mutext;
	int				simulation_end;
	long long		start_time;
}					t_environnement;

typedef struct t_node
{
	int			coder_id;
	long long	priority;
}				t_heap_node;

typedef struct t_heap
{
	t_heap_node	*data;
	int			size;
}				t_heap;

t_parsing_list	*parser(char **things);
long long		get_time_in_ms(void);
void			print_status(t_info_coder *coder, char *status);
void			*monitor_routine(void *arg);
void			*coder_routine(void *arg);
void			free_all(t_environnement *env);
int				is_prioritarian(t_info_coder *coder);
void			execute_compile(t_info_coder *coder);
void			get_ordered_dongles(t_info_coder *c, int *first, int *second);
void			minimal_heap_sort(t_info_coder *c, int n, char *s, int *r);

#endif