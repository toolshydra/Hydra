/*
 * src/analysis.c
 *
 * Copyright (C) 2008 Eduardo Valentin <edubezval@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include <analysis.h>

/*
 * print_task_model: prints task model info in a human readable way
 * @parameter tset: set of tasks
 * @parameter res: set of resources
 * @complexity: O(ntasks) + 2xO(nresources) + O(ntasks x nresources)
 */
static void print_task_model(struct task_set tset, struct res_set res)
{
	int i;

	printf("\n*********************\n");
	printf("* Modelo de Tarefas *\n");
	printf("*********************\n");
	printf("%02d tarefas, %02d recursos\n", tset.ntasks, res.nresources);
	printf("Tarefa        Prioridade        Computação        Meta\n");

	for (i = 0; i < tset.ntasks; i++)
		printf("T%2d        %2d                %05.2lf                %05.2lf\n", i + 1, i,
			tset.tasks[i].computation, tset.tasks[i].deadline);

	printf("\n**********************\n");
	printf("* Recursos do Modelo *\n");
	printf("**********************\n");
	printf("Task        ");
	for (i = 0; i < res.nresources; i++)
		printf("R%2d        ", i + 1);
	printf("\n");

	for (i = 0; i < tset.ntasks; i++) {
		int j;

		printf("T%2d        ", i + 1);

		for (j = 0; j < res.nresources; j++)
			printf("%05.2lf%%        ", tset.tasks[i].resources[j] * 100);
		printf("\n");

	}
	printf("Prioridades dos recursos\n");
	for (i = 0; i < res.nresources; i++)
		printf("C(R%2d) = %3d        ", i + 1, res.resource_priorities[i]);
	printf("\n");

}

/*
 * print_task_influencies: prints each task influence component
 * @parameter tset: set of tasks
 * @complexity: O(ntasks)
 */
static void print_task_influencies(struct task_set tset)
{
	int i;

	printf("\n**************\n");
	printf("* Influência *\n");
	printf("**************\n");
	printf("Tarefa        Bi        Ji        Ii\n");
	for (i = 0; i < tset.ntasks; i++) {
		printf("T%2d        %05.2lf        %05.2lf        %05.2lf\n", i + 1,
			tset.tasks[i].Ib, tset.tasks[i].Ij, tset.tasks[i].Ip);
	}
}

/*
 * print_task_analysis: prints each task computed data
 * @parameter tset: set of tasks
 * @complexity: O(ntasks)
 */
static void print_task_analysis(struct task_set tset)
{
	int i;

	printf("\n***********\n");
	printf("* Análise *\n");
	printf("***********\n");
	printf("Tarefa        Computação        Ii                Ri        Pi        (Pi - Ii)"
								"        (Pi - Ri)\n");

	for (i = 0; i < tset.ntasks; i++)
		printf("T%02d        %05.2lf                %05.2lf                %05.2lf                %05.2lf                %05.2lf                %05.2lf\n",
			i + 1,
			tset.tasks[i].computation,
			tset.tasks[i].Ip,
			response(tset.tasks[i]),
			tset.tasks[i].deadline,
			tset.tasks[i].deadline - tset.tasks[i].Ip,
			tset.tasks[i].deadline - response(tset.tasks[i]));

}

/*
 * evaluate_sample_response: evaluates the sample feasibility
 * @parameter tset: set of tasks
 * @parameter runtime: runtime information
 * @parameter spread: how good the sample can spread the time for tasks
 * @complexity: O(ntasks)
 */
static int evaluate_sample_response(struct task_set tset,
					struct run_info runtime, double *spread)
{
	int i, ok;
	char f[10];
	char di, de;
	double s = 0;

	if (runtime.list) {
		for (i = 0; i < tset.ntasks; i++)
			printf(" %6.2lf", tset.tasks[i].computation);

		printf("   [ ");
	}

	ok = 1;
	for (i = 0; i < tset.ntasks; i++) {
		double dx;
		if (tset.tasks[i].deadline < response(tset.tasks[i]) ||
			(tset.tasks[i].Ip < 0)) {
			di = '<';
			de = '>';
			ok = 0;
			dx = -1.0;
		} else {
			di = ' ';
			de = ' ';
			dx = tset.tasks[i].deadline - response(tset.tasks[i]);
		}

		sprintf(f, "%7.2lf", dx);
		if (runtime.list)
			printf("%c%7s%c ", di, f, de);

		s += dx;
	}

	if (runtime.list) {
		if (ok)
			printf("]   %-4s %7.2lf", "OK", s);
		else
			printf("]   %-4s %7.2lf", "NOT", -1.0);
		printf("\n");
	}

	if (ok)
		*spread = s;

	return ok;
}

/*
 * compute_resource_priorities: Compute resource priorities
 * @parameter tset: a set of tasks
 * @parameter res: array of integer which will be filled with priorities
 * @complexity: O(ntasks x nresources)
 */
static void compute_resource_priorities(struct task_set tset,
						struct res_set *res)
{
	int *p;
	int i, j;

	p = res->resource_priorities;
	for (i = 0; i < res->nresources; i++)
		p[i] = -1;

	for (i = 0; i < res->nresources; i++) {
		for (j = 0; j < tset.ntasks; j++) {
			if (tset.tasks[j].resources[i] > 0.0) {
				p[i] = j;
				break;
			}
		}
	}
}

/*
 * compute_exclusion_influency: Compute exclusion influency of each task
 * @parameter tset: set of tasks
 * @parameter res: set of integer with resource priorities
 * @complexity: O(nresources x ntasks ^ 2)
 */
static void compute_exclusion_influency(struct task_set tset,
						struct res_set res)
{
	int i, j, k;

	for (i = 0; i < tset.ntasks; i++) {
		tset.tasks[i].Ib = 0;
		for (k = i + 1; k < tset.ntasks; k++) {
			for (j = 0; j < res.nresources; j++) {
				/* If we have high priority */
				if (res.resource_priorities[j] <= i &&
					task_res_use(tset.tasks[k], j) >
					tset.tasks[i].Ib)
					tset.tasks[i].Ib =
					task_res_use(tset.tasks[k], j);
			}
		}
	}

}

/*
 * compute_precedence_influency: Compute precedence influency of each task
 * @parameter tset: set of tasks
 * @complexity: O(ntasks ^ 2)
 */

/* Usable AlmostEqual function */
int almostequal2s_complement(double A, double B, long long maxulps)
{
    long long aint;
    long long bint;
    long long intdiff = abs(aint - bint);

    /* Make aInt lexicographically ordered as a twos-complement int */
    aint = *(long long *)&A;
    if (aint < 0)
        aint = 0x80000000 - aint;

    /* Make bInt lexicographically ordered as a twos-complement int */
    bint = *(long long*)&B;
    if (bint < 0)
        bint = 0x80000000 - bint;

    intdiff = abs(aint - bint);
    if (intdiff <= maxulps)
        return 1;

    return 0;
}

static void compute_precedence_influency(struct task_set tset)
{
	int i, j;
	int success;
	double Ip, Ipa;

	for (i = 0; i < tset.ntasks; i++) {
		Ip = tset.tasks[i].computation + tset.tasks[i].Ib;
		success = 0;
		while (!success && Ip <= tset.tasks[i].deadline) {
			Ipa = Ip;
			Ip = tset.tasks[i].computation + tset.tasks[i].Ib;
			for (j = i - 1; j >= 0; j--)
				Ip += precedence_influency(tset.tasks[j], Ipa);

			if (almostequal2s_complement(Ip, Ipa, 1 << 22))
				success = 1;
		}
		if (success)
			tset.tasks[i].Ip = Ip;
		else
			tset.tasks[i].Ip = -1;
	}
}

/*
 * propagate: propagate index update
 * @parameter last: index to start to update
 * @parameter ind: array of indexes
 * @parameter limits: array of index limits
 */
static int propagate(int last, int *ind, int *limits)
{
	int i;

	i = last;
	while (++ind[i] >= limits[i]) {
		if (ind[0] == limits[0])
			break;
		ind[i] = 0;
		i--;
	}
	return i;
}

/*
 * start_drop: binary search for an optimal start point
 * @parameter tset: set of tasks
 * @parameter freqs: set of double with available frequencies
 * @parameter res: set of resources
 * @parameter runtime: runtime info
 * @parameter ind: an array of indexes to be filled up
 * @parameter limits: an array of limits to be filled up
 * @parameter done: an array of done indexes to be filled up
 * @parameter stat: computed stats results
 * @complexity: O(log(nfrequencies)) + O(ntask)
 */
static int start_drop(struct task_set tset, struct freq_set freqs,
			struct res_set *res, struct run_info runtime, int *ind,
			int *limits, int *done, struct results *stat)
{
	int i, f, m, pass, last, start;
	double spread;

	f = 0;
	last = freqs.nfrequencies - 1;
	while (f <= last) {
		stat->total++;
		m = (f + last) / 2;
		for (i = 0; i < tset.ntasks; i++) {
			ind[i] = m;
			tset.tasks[i].computation = tset.tasks[i].wcec /
						freqs.frequencies[ind[i]];
		}
		done[m] = 1;
		if (runtime.list)
			printf("%03d -", stat->total, m);

		compute_sample_analysis(tset, res, runtime);
		pass = evaluate_sample_response(tset, runtime, &spread);

		stat->success += pass;

		if (!pass) {
			last = m - 1;
		} else {
			f = m + 1;
			start = m;
			if (spread < stat->best) {
				stat->best = spread;
				for (i = 0; i < tset.ntasks; i++)
					stat->best_index[i] = ind[i];
			}
		}
	}

	for (i = 0; i < tset.ntasks; i++)
		ind[i] = start;

	return propagate(tset.ntasks - 1, ind, limits);
}

/*
 * compute_initial_limits: Compute limits of start point
 * @parameter tset: set of tasks
 * @parameter freqs: set of frequencies (available frequencies)
 * @parameter runtime: runtime info
 * @parameter start_limits: array of integer which will be filled with limits
 * @complexity: O(ntasks x nfrequencies)
 */
int compute_initial_limits(struct task_set tset, struct freq_set freqs,
				struct run_info runtime, int **start_limits)
{
	int i, j;
	int *limits;
	int err = 0;

	limits = *start_limits;
	for (i = 0 ; i < tset.ntasks; i++)
		limits[i] = freqs.nfrequencies;

	if (!runtime.best_initial_limits)
		goto exit;

	for (i = freqs.nfrequencies - 1; i >= 0 ; i--) {
		for (j = 0; j < tset.ntasks; j++) {
			tset.tasks[j].computation = tset.tasks[j].wcec /
							freqs.frequencies[i];
			if (tset.tasks[j].computation > tset.tasks[j].deadline)
				limits[j] = i;
		}
	}

exit:
	*start_limits = limits;
	return err;
}

/*
 * compute_sample_analysis: compute influency for each task
 * @parameter tset: set of tasks
 * @parameter res: set of resources
 * @parameter runtime: determines if output will be verbose
 * @complexity: O(nresources x ntasks ^ 2)
 */
void compute_sample_analysis(struct task_set tset, struct res_set *res,
				struct run_info runtime)
{

	/* O(ntasks x nresources) */
	compute_resource_priorities(tset, res);
	if (runtime.verbose)
		/* O(ntasks) + 2xO(nresources) + O(ntasks x nresources) */
		print_task_model(tset, *res);

	/* O(nresources x ntasks ^ 2) */
	compute_exclusion_influency(tset, *res);
	/* O(ntasks ^ 2) */
	compute_precedence_influency(tset);

	if (runtime.verbose) {
		/* O(ntasks) */
		print_task_influencies(tset);

		/* O(ntasks) */
		print_task_analysis(tset);
	}
}

/*
 * enumerate_samples: go enumerating samples and branch when possible
 * @parameter tset: set of tasks
 * @parameter freqs: set of frequencies
 * @parameter res: set of resources
 * @parameter limits: array of integer with initial frequency limits
 * @parameter runtime: runtime info
 * @parameter stat: struct to store statistic info
 * @complexity: O(nfrequencies ^ ntasks) x O(nresources x ntasks ^ 2)
 */
static int *ind = NULL;
static int *done = NULL;

int enumerate_samples(struct task_set tset, struct freq_set freqs,
			struct res_set *res, int *limits,
			struct run_info runtime, struct results *stat)
{

	int i, last;
	int pass;
	int jump_done;
	double spread;

	for (i = 0; i < tset.ntasks; i++)
		ind[i] = 0;

	for (i = 0; i < freqs.nfrequencies; i++)
		done[i] = 0;

	for (i = 0; i < tset.ntasks; i++)
		stat->best_index[i] = 0;

	stat->total = 0;
	stat->success = 0;
	last = tset.ntasks - 1;
	stat->best = HUGE_VAL;

	if (runtime.best_start)
		start_drop(tset, freqs, res, runtime, ind, limits, done, stat);

	while (ind[0] < limits[0]) {
		if (done[ind[0]]) {
			jump_done = 1;
			for (i = 1; i < tset.ntasks; i++)
				if (ind[i] != ind[0]) {
					jump_done = 0;
					break;
				}
			if (jump_done) {
				last = propagate(tset.ntasks - 1, ind, limits);
				continue;
			}
		}
		++stat->total;
		if (runtime.list)
			printf("%03d -", stat->total);
		for (i = 0; i < tset.ntasks; i++) {
			tset.tasks[i].computation = tset.tasks[i].wcec /
						freqs.frequencies[ind[i]];
		}

		compute_sample_analysis(tset, res, runtime);
		pass = evaluate_sample_response(tset, runtime, &spread);
		stat->success += pass;

		if (pass && spread < stat->best) {
			stat->best = spread;
			for (i = 0; i < tset.ntasks; i++)
				stat->best_index[i] = ind[i];
		}

		if (runtime.jump_samples && !pass) {
			/* as we found the last, we can re-start it */
			ind[last] = 0;

			last--;
			/* check if we reached the end */
			if (last < 0)
				break;
			/* we then go to the next level */
			last = propagate(last, ind, limits);
			continue;
		}
		last = propagate(tset.ntasks - 1, ind, limits);
	}

	return 0;
}

int enumeration_init(struct task_set *tset, struct freq_set *freqs,
			struct results *stat,
			struct res_set *res,
			int **limits)
{
	int i;

	tset->tasks = (struct task *)malloc(tset->ntasks *
						sizeof(*tset->tasks));

	if (!tset->tasks) {
		printf("Could not allocate memory for %d tasks.\n",
							tset->ntasks);
		return -ENOMEM;
	}

	*limits = (int *)malloc(tset->ntasks * sizeof(int));
	if (!*limits) {
		printf("Could not allocate memory for indices.\n");
		return -ENOMEM;
	}

	ind = (int *)malloc(tset->ntasks * sizeof(int));
	if (!ind) {
		printf("Could not allocate memory for indices.\n");
		return -ENOMEM;
	}

	stat->best_index = (int *)malloc(tset->ntasks * sizeof(int));
	if (!stat->best_index) {
		printf("Could not allocate memory for indices.\n");
		return -ENOMEM;
	}

	for (i = 0; i < tset->ntasks; i++) {
		struct task *t = tset->tasks + i;

		t->resources = (double *)malloc(sizeof(double) * res->nresources);
		if (!t->resources) {
			printf("Could not allocate memory for array\n");
			return -ENOMEM;
		}
	}

	res->resource_priorities = (int *)malloc(sizeof(int) * res->nresources);
	if (!res->resource_priorities) {
		printf("Could not allocate memory for resource priorities.\n");
		return -ENOMEM;
	}

	freqs->frequencies = (double *)malloc(sizeof(double) * freqs->nfrequencies);
	if (!freqs->frequencies) {
		printf("Could not allocate memory for array\n");
		return -ENOMEM;
	}

	freqs->voltages = (double *)malloc(sizeof(double) * freqs->nfrequencies);
	if (!freqs->voltages) {
		printf("Could not allocate memory for array\n");
		return -ENOMEM;
	}

	done = (int *)malloc(freqs->nfrequencies * sizeof(int));
	if (!done) {
		printf("Could not allocate memory for done array.\n");
		return -ENOMEM;
	}

	return 0;
}

int enumeration_cleanup(struct task_set *tset, struct freq_set *freqs,
			struct results *stat,
			struct res_set *res,
			int *limits)
{
	int i;

	for (i = 0; i < tset->ntasks; i++) {
		struct task *t = tset->tasks + i;

		if (t->resources) {
			free(t->resources);
			t->resources = NULL;
		}
	}

	if (tset->tasks) {
		free(tset->tasks);
		tset->tasks = NULL;
	}

	if (limits) {
		free(limits);
		limits = NULL;
	}

	if (ind) {
		free(ind);
		ind = NULL;
	}

	if (freqs->frequencies) {
		free(freqs->frequencies);
		freqs->frequencies = NULL;
	}

	if (freqs->voltages) {
		free(freqs->voltages);
		freqs->voltages = NULL;
	}

	if (done) {
		free(done);
		done = NULL;
	}

	if (stat->best_index) {
		free(stat->best_index);
		stat->best_index = NULL;
	}

	if (res->resource_priorities) {
		free(res->resource_priorities);
		res->resource_priorities = NULL;
	}
}

