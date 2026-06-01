/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 18:09:59 by evvan             #+#    #+#             */
/*   Updated: 2026/06/01 19:06:12 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_numeric(char *str)
{
	int	i;

	i = 0;
	if (!str || !str[0])
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	is_valid_values(char **things)
{
	int	i;

	i = 1;
	if (!things || !things[1] || !things[2] || !things[3] || !things[4]
		|| !things[5] || !things[6] || !things[7] || !things[8])
		return (0);
	while (i <= 7)
	{
		if (!is_numeric(things[i]) || atoi(things[i]) < 0)
			return (0);
		i++;
	}
	if (strcmp(things[8], "fifo") != 0 && strcmp(things[8], "edf") != 0)
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
