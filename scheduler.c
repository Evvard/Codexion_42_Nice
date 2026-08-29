/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 21:30:00 by evvan             #+#    #+#             */
/*   Updated: 2026/06/08 21:30:00 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_ready(t_environnement *e, int idx)
{
	int			l;
	int			r;
	long long	now;

	l = e->coder[idx].left_dongle;
	r = e->coder[idx].right_dongle;
	now = get_time_in_ms();
	if (e->dongle_held[l] || e->dongle_held[r])
		return (0);
	if (now < e->dongle_cooldown_ends[l] || now < e->dongle_cooldown_ends[r])
		return (0);
	return (1);
}

static int	can_grant(t_info_coder *c)
{
	t_environnement	*e;
	int				ln;
	int				rn;

	e = c->env;
	if (!is_ready(e, c->nb_of_coder - 1))
		return (0);
	if (heap_peek_id(e->queue) == c->nb_of_coder - 1)
		return (1);
	ln = (c->nb_of_coder - 2 + e->params->number_of_coder)
		% e->params->number_of_coder;
	rn = c->nb_of_coder % e->params->number_of_coder;
	if (e->coder[ln].waiting && e->queue->cmp(&e->coder[ln].req, &c->req)
		&& is_ready(e, ln))
		return (0);
	if (e->coder[rn].waiting && e->queue->cmp(&e->coder[rn].req, &c->req)
		&& is_ready(e, rn))
		return (0);
	return (1);
}

static void	do_grant(t_info_coder *c)
{
	t_environnement	*e;

	e = c->env;
	e->waiters++;
	while (!e->simulation_end && !can_grant(c))
		block_until_signal(e, compute_timeout(c));
	e->waiters--;
	c->waiting = 0;
	heap_remove(e->queue, c->nb_of_coder - 1);
	if (e->simulation_end)
		return ;
	e->dongle_held[c->left_dongle] = 1;
	e->dongle_held[c->right_dongle] = 1;
	c->last_compile_start = get_time_in_ms();
}

void	request_dongles(t_info_coder *c)
{
	t_environnement	*e;

	e = c->env;
	pthread_mutex_lock(&e->sched_lock);
	c->req.coder_id = c->nb_of_coder - 1;
	if (c->compile_count == 0)
		c->req.seq = c->nb_of_coder - 1;
	else
		c->req.seq = e->seq_counter++;
	c->req.deadline = c->last_compile_start + e->params->time_to_burnout;
	c->waiting = 1;
	heap_push(e->queue, c->req);
	do_grant(c);
	pthread_mutex_unlock(&e->sched_lock);
}
