/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 18:09:59 by evvan             #+#    #+#             */
/*   Updated: 2026/05/25 20:12:20 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_valid_values(char **things)
{
	if (!things || !things[1] || !things[2] || !things[3] || !things[4]
		|| !things[5] || !things[6] || !things[7] || !things[8])
		return (0);
	if (!atoi(things[1]) || !atoi(things[2]) || !atoi(things[3]))
		return (0);
	if (!atoi(things[4]) || !atoi(things[5]) || !atoi(things[6]))
		return (0);
	if (!atoi(things[7]))
		return (0);
	if (!(strcmp(things[8], "fifo") == 0 || strcmp(things[8], "edf") == 0))
		return (0);
	return (1);
}

t_parsing_list	*parser(char **things)
{
	t_parsing_list	*result;

	result = malloc(sizeof(t_parsing_list));
	if (!result)
		return (NULL);
	if (is_valid_values(things))
	{
		result->number_of_coder = atoi(things[1]);
		result->time_to_burnout = atoi(things[2]);
		result->time_to_compile = atoi(things[3]);
		result->time_to_debug = atoi(things[4]);
		result->time_to_refactor = atoi(things[5]);
		result ->number_of_compiles_required = atoi(things[6]);
		result->dongle_cooldown = atoi(things[7]);
		result->scheduler = strdup(things[8]);
	}
	else
	{
		free(result);
		return (NULL);
	}
	return (result);
}
