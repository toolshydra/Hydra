/*
 * include/analysis.h
 *
 * Copyright (C) 2008 Eduardo Valentin <edubezval@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef ANALYSIS_H
#define ANALYSIS_H

/* Task data */
struct task {
	float deadline;
	float wcec;
	float computation;
	float Ip;
	float Ib;
	float Ij;
	float *resources;
};

#define	task_res_use(t, i)		(t.resources[i] * t.computation)
#define precedence_influency(t, w)	(ceil((w + t.Ij) / t.deadline) * \
							t.computation)
#define response(t)			(t.Ip + t.Ij)

/* Sets */
struct task_set {
	int ntasks;
	struct task *tasks;
};

struct freq_set {
	int nfrequencies;
	float *frequencies;
};

struct res_set {
	int nresources;
	int *resource_priorities;
};

/* Runtime data */
struct run_info {
	int summary:1;			/* print a summary in the end */
	int verbose:1;			/* verbose execution */
	int list:1;			/* list samples */
	int best_start:1;		/* comput best start point */
	int jump_samples:1;		/* jump useless samples */
	int best_initial_limits:1;	/* compute best initial freq limits */
};

/* Results */
struct results {
	struct timeval s;
	struct timeval e;
	int success;
	int total;
	float best;
	int *best_index;
};

int compute_initial_limits(struct task_set tset, struct freq_set freqs,
				struct run_info runtime, int **start_limits);
void compute_sample_analysis(struct task_set tset, struct res_set res,
				struct run_info runtime);
int enumerate_samples(struct task_set tset, struct freq_set freqs,
			struct res_set res, int *limits,
			struct run_info runtime, struct results *stat);
#endif
