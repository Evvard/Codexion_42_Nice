/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 18:09:59 by evvan             #+#    #+#             */
/*   Updated: 2026/05/12 10:35:52 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_valid_values(char **things)
{
	if (!atoi(things[1]) || !atof(things[2]) || !atof(things[3]))
		return (0);
	if (!atof(things[4]) || !atof(things[5]) || !atoi(things[6]))
		return (0);
	if (!atof(things[7]))
		return (0);
	if (!(strcmp(things[8], "fifo") == 0 || strcmp(things[8], "edf") == 0))
		return (0);
	return (1);
}

t_info	parser(char **things)
{
	t_info	*result;

	result = malloc(sizeof(t_info));
	if (is_valid_values(things))
	{
		result->number_of_coder = atoi(things[1]);
		result->time_to_burnout = atof(things[2]);
		result->time_to_compile = atof(things[3]);
		result->time_to_debug = atof(things[4]);
		result->time_to_refactor = atof(things[5]);
		result ->number_of_compiles_required = atoi(things[6]);
		result->dongle_cooldown = atof(things[7]);
		result->scheduler = things[8];
	}
	else
		return ;
	return (*result);
}
