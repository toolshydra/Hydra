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
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter nresources: integer which represents the number of resources
 * @parameter resource_priorities: array of integer with resources priorities
 * @complexity: O(ntasks) + 2xO(nresources) + O(ntasks x nresources)
 */
void print_task_model(int ntasks, struct task *tasks,
			int nresources, int *resource_priorities)
{
	int i;

	printf("\n**************\n");
	printf("* Task Model *\n");
	printf("**************\n");
	printf("%02d tasks, %02d resources\n", ntasks, nresources);
	printf("Task\tPriority\tComputation\tDeadline\n");

	for (i = 0; i < ntasks; i++)
		printf("T%d\t%d\t\t%.2f\t\t%.2f\n", i + 1, i,
			tasks[i].computation, tasks[i].deadline);

	printf("\n******************\n");
	printf("* Resource Model *\n");
	printf("******************\n");
	printf("Task\t");
	for (i = 0; i < nresources; i++)
		printf("R%d\t", i + 1);
	printf("\n");

	for (i = 0; i < ntasks; i++) {
		int j;

		printf("T%d\t", i + 1);

		for (j = 0; j < nresources; j++)
			printf("%.2f\%\t", tasks[i].resources[j] * 100);
		printf("\n");

	}
	printf("Resource priorities\n");
	for (i = 0; i < nresources; i++)
		printf("C(R%d) = %d\t", i + 1, resource_priorities[i]);
	printf("\n");

}

/*
 * print_task_influencies: prints each task influence component
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @complexity: O(ntasks)
 */
void print_task_influencies(int ntasks, struct task *tasks)
{
	int i;

	printf("\n*************\n");
	printf("* Influency *\n");
	printf("*************\n");
	printf("Task\tIp\tIb\tIj\tI\n");
	for (i = 0; i < ntasks; i++) {
		float I = tasks[i].Ip + tasks[i].Ib + tasks[i].Ij;
		printf("T%d\t%.2f\t%.2f\t%.2f\t%.2f\n", i + 1, tasks[i].Ip,
			tasks[i].Ib, tasks[i].Ij, I);
	}
}

/*
 * print_task_analysis: prints each task computed data
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter verbose: determines is the output will be verbose
 * @complexity: O(ntasks)
 */
void print_task_analysis(int ntasks, struct task *tasks)
{
	int i;

	printf("\n************\n");
	printf("* Analysis *\n");
	printf("************\n");
	printf("Task\tComputation\tI\t\tResponse\tDeadline\tK (D - I)"
								"\tD - R\n");

	for (i = 0; i < ntasks; i++)
		printf("T%d\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\n",
				i + 1,
				tasks[i].computation,
				0,
				response(tasks[i]),
				tasks[i].deadline,
				0,
				tasks[i].deadline - response(tasks[i]));

}

/*
 * evaluate_sample_response: evaluates the sample viability
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter verbose: determines is the output will be verbose
 * @complexity: O(ntasks)
 */
int evaluate_sample_response(int ntasks, struct task *tasks)
{
	int i, ok;
	char f[10];
	char di, de;
	float s = 0;

	for (i = 0; i < ntasks; i++)
		printf(" %6.2f", tasks[i].computation);

	printf("\t[ ");

	ok = 1;
	for (i = 0; i < ntasks; i++) {
		float dx;
		if (tasks[i].deadline < response(tasks[i]) ||
			(tasks[i].Ip < 0)) {
			di = '<';
			de = '>';
			ok = 0;
			dx = -1.0;
		} else {
			di = ' ';
			de = ' ';
			dx = tasks[i].deadline - response(tasks[i]);
		}

		sprintf(f, "%7.2f", dx);
		printf("%c%7s%c ", di, f, de);

		s += dx;
	}

	if (ok)
		printf("]\t%-4s %7.2f", "OK", s);
	else
		printf("]\t%-4s %7.2f", "NOT", -1.0);
	printf("\n");

	return ok;
}

/*
 * compute_initial_limits: Compute limits of start point
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter nfrequencies: integer which represents the number of frequencies
 * @parameter nfrequencies: represents if initial drop will be done
 * @parameter start_limits: array of integer which will be filled with limits
 * @complexity: O(ntasks x nfrequencies)
 */
int compute_initial_limits(int ntasks, struct task *tasks, int nfrequencies,
				float *frequencies, int drop,
				int **start_limits)
{
	int i, j;
	int *limits;
	int err = 0;

	limits = malloc(ntasks * sizeof(int));
	if (!limits) {
		printf("Could not allocate memory for indices.\n");
		err = -ENOMEM;
		goto exit;
	}

	for (i = 0 ; i < ntasks; i++)
		limits[i] = nfrequencies;

	if (!drop)
		goto exit;

	for (i = nfrequencies - 1; i >= 0 ; i--) {
		for (j = 0; j < ntasks; j++) {
			tasks[j].computation = tasks[j].wcec / frequencies[i];
			if (tasks[j].computation > tasks[j].deadline)
				limits[j] = i;
		}
	}

exit:
	*start_limits = limits;
	return err;
}

/*
 * compute_resource_priorities: Compute resource priorities
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter nresources: integer which represents the number of resources
 * @parameter priorities: array of integer which will be filled with priorities
 * @complexity: O(ntasks x nresources)
 */
void compute_resource_priorities(int ntasks, struct task *tasks,
					int nresources, int **priorities)
{
	int *p;
	int i, j;

	*priorities = malloc(sizeof(int) * nresources);
	p = *priorities;
	memset(p, -1, nresources);

	for (i = 0; i < nresources; i++) {
		for (j = 0; j < ntasks; j++) {
			if (tasks[j].resources[i] > 0.0) {
				p[i] = j;
				break;
			}
		}
	}
}

/*
 * compute_exclusion_influency: Compute exclusion influency of each task
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter nresources: integer which represents the number of resources
 * @parameter priorities: array of integer with resource priorities
 * @complexity: O(nresources x ntasks ^ 2)
 */
void compute_exclusion_influency(int ntasks, struct task *tasks,
					int nresources, int *priorities)
{
	int i, j, k;

	for (i = 0; i < ntasks; i++) {
		tasks[i].Ib = 0;
		for (k = i + 1; k < ntasks; k++) {
			for (j = 0; j < nresources; j++) {
				/* If we have high priority */
				if (priorities[j] <= i &&
					task_res_use(tasks[k], j) > tasks[i].Ib)
					tasks[i].Ib = task_res_use(tasks[k], j);
			}
		}
	}

}

/*
 * compute_precedence_influency: Compute precedence influency of each task
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @complexity: O(ntasks ^ 2)
 */
void compute_precedence_influency(int ntasks, struct task *tasks)
{
	int i, j;
	int success;
	float Ip, Ipa;

	for (i = 0; i < ntasks; i++) {
		Ip = tasks[i].computation + tasks[i].Ib;
		success = 0;
		while (!success && Ip <= tasks[i].deadline) {
			Ipa = Ip;
			Ip = tasks[i].computation + tasks[i].Ib;
			for (j = i - 1; j >= 0; j--)
				Ip += precedence_influency(tasks[j], Ipa);

			if (Ip == Ipa)
				success = 1;
		}
		if (success)
			tasks[i].Ip = Ip;
		else
			tasks[i].Ip = -1;
	}
}

/*
 * compute_sample_analysis: compute influency for each task
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter nresources: integer which represents the number of resources
 * @parameter verbose: determines if output will be verbose
 * @complexity: O(nresources x ntasks ^ 2)
 */
void compute_sample_analysis(int ntasks, struct task *tasks,
				int nresources, int verbose)
{
	int *resource_priorities;

	/* O(ntasks x nresources) */
	compute_resource_priorities(ntasks, tasks, nresources,
						&resource_priorities);
	if (verbose)
	/* O(ntasks) + 2xO(nresources) + O(ntasks x nresources) */
		print_task_model(ntasks, tasks, nresources,
						resource_priorities);

	/* O(nresources x ntasks ^ 2) */
	compute_exclusion_influency(ntasks, tasks, nresources,
							resource_priorities);
	/* O(ntasks ^ 2) */
	compute_precedence_influency(ntasks, tasks);

	if (verbose) {
		/* O(ntasks) */
		print_task_influencies(ntasks, tasks);

		/* O(ntasks) */
		print_task_analysis(ntasks, tasks);
	}
}

/*
 * compute_sample_analysis: compute influency for each task
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter nfrequencies: integer which represents the number of frequencies
 * @parameter frequencies: array of float with available frequencies
 * @parameter nresources: integer which represents the number of resources
 * @parameter resource_priorities: array of integer with resource priorities
 * @parameter limits: array of integer with initial frequency limits
 * @parameter verbose: determines if output will be verbose
 * @parameter success: output parameter with number of feasible samples
 * @parameter total: output parameter with total evaluated samples
 * @complexity: O(nfrequencies ^ ntasks) x O(nresources x ntasks ^ 2)
 */
int enumerate_samples(int ntasks, struct task *tasks, int nfrequencies,
			float *frequencies, int nresources,
			int *resource_priorities, int *limits, int verbose,
			int *success, int *total)
{
	int i;
	int *ind;

	ind = malloc(ntasks * sizeof(int));
	if (!ind) {
		printf("Could not allocate memory for indices.\n");
		return -ENOMEM;
	}
	memset(ind, ntasks, 0);

	*total = 1;
	*success = 0;
	while (ind[0] < limits[0]) {

		printf("%03d -", (*total)++);
		for (i = 0; i < ntasks; i++) {
			tasks[i].computation = tasks[i].wcec /
							frequencies[ind[i]];
		}

		compute_sample_analysis(ntasks, tasks, nresources, verbose);
		*success += evaluate_sample_response(ntasks, tasks);

		i = ntasks - 1;
		while (++ind[i] >= limits[i]) {
			if (ind[0] == limits[0])
				break;
			ind[i] = 0;
			i--;
		}
	}

	return 0;

}


