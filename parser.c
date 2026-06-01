/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eolivier <eolivier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 18:09:59 by evvan             #+#    #+#             */
/*   Updated: 2026/06/01 12:00:00 by eolivier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_numeric_string(char *str)
{
	int	i;

	i = 0;
	if (!str || str[0] == '\0')
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	validate_args(char **things)
{
	int	i;

	if (!things || !things[1] || !things[2] || !things[3] || !things[4]
		|| !things[5] || !things[6] || !things[7] || !things[8])
		return (0);
	i = 1;
	while (i <= 7)
	{
		if (!is_numeric_string(things[i]))
			return (0);
		i++;
	}
	if (atoi(things[1]) <= 0 || !(*things[8]))
		return (0);
	if (strcmp(things[8], "fifo") != 0 && strcmp(things[8], "edf") != 0)
		return (0);
	return (1);
}

t_parsing_list	*parser(char **things)
{
	t_parsing_list	*list;

	if (!validate_args(things))
		return (NULL);
	list = malloc(sizeof(t_parsing_list));
	if (!list)
		return (NULL);
	list->number_of_coder = atoi(things[1]);
	list->time_to_burnout = atoi(things[2]);
	list->time_to_compile = atoi(things[3]);
	list->time_to_debug = atoi(things[4]);
	list->time_to_refactor = atoi(things[5]);
	list->number_of_compiles_required = atoi(things[6]);
	list->dongle_cooldown = atoi(things[7]);
	list->scheduler = strdup(things[8]);
	return (list);
}
