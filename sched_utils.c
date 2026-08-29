/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sched_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: evvan <evvan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/15 17:30:00 by evvan             #+#    #+#             */
/*   Updated: 2026/06/15 17:30:00 by evvan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	compute_timeout(t_info_coder *c)
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

void	block_until_signal(t_environnement *e, long long ms)
{
	struct timespec	ts;

	if (ms < 0)
		pthread_cond_wait(&e->queue_cond, &e->sched_lock);
	else
	{
		ts.tv_sec = ms / 1000;
		ts.tv_nsec = (ms % 1000) * 1000000;
		pthread_cond_timedwait(&e->queue_cond, &e->sched_lock, &ts);
	}
}
