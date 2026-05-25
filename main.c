/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:30:24 by evvan             #+#    #+#             */
/*   Updated: 2026/05/25 18:20:42 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int main(int ac, char *av[])
{
	t_parsing_list  *argument;

	if (ac == 9)
	{
		argument = parser(av); 
		if (!argument)
		{
			printf("Erreur de parsing ou arguments invalides.\n");
			return (1);
		}
		
		printf("Coder count: %d, Scheduler: %s\n", argument->number_of_coder, argument->scheduler);
		
		free(argument->scheduler);
		free(argument);
	}
	else
	{
		printf("Usage: %s [coder] [burnout] [compile] [debug] [refactor] [compiles_req] [cooldown] [scheduler]\n", av[0]);
		return (1);
	}
	return (0);
}



