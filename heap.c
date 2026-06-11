/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 21:30:00 by evvan             #+#    #+#             */
/*   Updated: 2026/06/08 21:30:00 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	sift_up(t_heap *h, int i)
{
	int			parent;
	t_request	tmp;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!h->cmp(&h->array[i], &h->array[parent]))
			break ;
		tmp = h->array[i];
		h->array[i] = h->array[parent];
		h->array[parent] = tmp;
		i = parent;
	}
}

static void	sift_down(t_heap *h, int i)
{
	int			best;
	int			left;
	int			right;
	t_request	tmp;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		best = i;
		if (left < h->size && h->cmp(&h->array[left], &h->array[best]))
			best = left;
		if (right < h->size && h->cmp(&h->array[right], &h->array[best]))
			best = right;
		if (best == i)
			break ;
		tmp = h->array[i];
		h->array[i] = h->array[best];
		h->array[best] = tmp;
		i = best;
	}
}

void	heap_push(t_heap *h, t_request req)
{
	h->array[h->size] = req;
	h->size++;
	sift_up(h, h->size - 1);
}

void	heap_remove(t_heap *h, int coder_id)
{
	int	i;

	i = 0;
	while (i < h->size && h->array[i].coder_id != coder_id)
		i++;
	if (i >= h->size)
		return ;
	h->size--;
	h->array[i] = h->array[h->size];
	if (i < h->size)
	{
		if (i > 0 && h->cmp(&h->array[i], &h->array[(i - 1) / 2]))
			sift_up(h, i);
		else
			sift_down(h, i);
	}
}
