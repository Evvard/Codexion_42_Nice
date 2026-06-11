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

static long long	compute_timeout(t_info_coder *c)
{
	t_environnement	*e;
	long long		now;
	long long		best;
	long long		left;
	long long		right;

	e = c->env;
	now = get_time_in_ms();
	left = e->dongle_cooldown_ends[c->left_dongle];
	right = e->dongle_cooldown_ends[c->right_dongle];
	best = -1;
	if (left > now)
		best = left;
	if (right > now && (best < 0 || right < best))
		best = right;
	return (best);
}

static void	do_grant(t_info_coder *c)
{
	t_environnement	*e;
	struct timespec	ts;
	long long		ms;

	e = c->env;
	while (!e->simulation_end && !can_grant(c))
	{
		ms = compute_timeout(c);
		if (ms < 0)
			pthread_cond_wait(&e->queue_cond, &e->sched_lock);
		else
		{
			ts.tv_sec = ms / 1000;
			ts.tv_nsec = (ms % 1000) * 1000000;
			pthread_cond_timedwait(&e->queue_cond, &e->sched_lock, &ts);
		}
	}
	c->waiting = 0;
	heap_remove(e->queue, c->nb_of_coder - 1);
	if (e->simulation_end)
		return ;
	e->dongle_held[c->left_dongle] = 1;
	e->dongle_held[c->right_dongle] = 1;
	c->last_compile_start = get_time_in_ms();
	pthread_cond_broadcast(&e->queue_cond);
}

void	request_dongles(t_info_coder *c)
{
	t_environnement	*e;

	e = c->env;
	pthread_mutex_lock(&e->sched_lock);
	c->req.coder_id = c->nb_of_coder - 1;
	c->req.seq = e->seq_counter++;
	c->req.deadline = c->last_compile_start + e->params->time_to_burnout;
	c->waiting = 1;
	heap_push(e->queue, c->req);
	pthread_cond_broadcast(&e->queue_cond);
	do_grant(c);
	pthread_mutex_unlock(&e->sched_lock);
}
