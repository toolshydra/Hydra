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

#define N_TRIES	5
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
#define response(t)		(t.Ip + t.Ij)

void print_task_model(int ntasks, struct task *tasks,
			int nresources, int *resource_priorities);
void print_task_influencies(int ntasks, struct task *tasks);
void print_task_analysis(int ntasks, struct task *tasks);
int evaluate_sample_response(int ntasks, struct task *tasks, int list);
int compute_initial_limits(int ntasks, struct task *tasks, int nfrequencies,
				float *frequencies, int drop,
				int **start_limits);
void compute_resource_priorities(int ntasks, struct task *tasks,
					int nresources, int **priorities);
void compute_exclusion_influency(int ntasks, struct task *tasks,
					int nresources, int *priorities);
void compute_precedence_influency(int ntasks, struct task *tasks);
void compute_sample_analysis(int ntasks, struct task *tasks,
				int nresources, int verbose);
int enumerate_samples(int ntasks, struct task *tasks, int nfrequencies,
			float *frequencies, int nresources,
			int *resource_priorities, int *limits, int verbose,
			int list, int jump, int *success, int *total);
#endif
