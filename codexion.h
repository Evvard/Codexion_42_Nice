/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:30:22 by evvan             #+#    #+#             */
/*   Updated: 2026/05/18 17:18:26 by evvan            ###   ########.fr       */
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

typedef struct t_list
{
	int		number_of_coder;
	float	time_to_burnout;
	float	time_to_compile;
	float	time_to_debug;
	float	time_to_refactor;
	int		number_of_compiles_required;
	float	dongle_cooldown;
	char	*scheduler;
}			parsing_list;





parsing_list	*parser(char **things);

#endif