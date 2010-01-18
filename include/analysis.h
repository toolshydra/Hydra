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

#define NTRIES	1000
/* Task data */
struct task {
	double deadline;
	double wcec;
	double computation;
	double Ip;
	double Ib;
	double Ij;
	double *resources;
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
	double *frequencies;
	double *voltages;
};

struct res_set {
	int nresources;
	int *resource_priorities;
};

/* Runtime data */
struct run_info {
	int summary:1;			/* print a summary in the end */
	int tabular:1;			/* print a summary in one line */
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
	double best;
	int *best_index;
};

int compute_initial_limits(struct task_set tset, struct freq_set freqs,
				struct run_info runtime, int **start_limits);
void compute_sample_analysis(struct task_set tset, struct res_set *res,
				struct run_info runtime);
int enumerate_samples(struct task_set tset, struct freq_set freqs,
			struct res_set *res, int *limits,
			struct run_info runtime, struct results *stat);
int enumeration_init(struct task_set *tset, struct freq_set *freqs,
			struct results *stat,
			struct res_set *res,
			int **limits);
int enumeration_cleanup(struct task_set *tset, struct freq_set *freqs,
			struct results *stat,
			struct res_set *res,
			int *limits);
#endif
