/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:30:22 by evvan             #+#    #+#             */
/*   Updated: 2026/05/12 09:08:08 by evvan            ###   ########.fr       */
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

typedef struct s_info
{
	int		number_of_coder;
	float	time_to_burnout;
	float	time_to_compile;
	float	time_to_debug;
	float	time_to_refactor;
	int		number_of_compiles_required;
	float	dongle_cooldown;
	char	scheduler;
}			t_info;


typedef struct t_list
{
	void	*content;
	struct	t_list;
}			parsing_list;


t_info	parser(char **things)

#endif