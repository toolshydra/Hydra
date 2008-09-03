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
 * print_task_analysis: prints each task influence component
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter verbose: determines is the output will be verbose
 * @complexity: O(ntasks)
 */
void print_task_analysis(int ntasks, struct task *tasks, int verbose)
{
	int i, ok;
	char f[10];
	char di, de;
	float s = 0;

	if (verbose) {
		printf("\n************\n");
		printf("* Analysis *\n");
		printf("************\n");
		printf("Task\tComputation\tI\t\tResponse\tDeadline\tK (D - I)"
								"\tD - R\n");
	} else
		printf("\t[ ");

	ok = 1;
	for (i = 0; i < ntasks; i++) {
		float R = tasks[i].Ip + tasks[i].Ij;
		if (verbose)
			printf("T%d\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f"
				"\t\t%.2f\n", i + 1,
				tasks[i].computation, 0,
				R,
				tasks[i].deadline,
				0,
				tasks[i].deadline - R);
		if (tasks[i].deadline < R) {
			di = '<';
			de = '>';
			ok = 0;
		} else {
			di = ' ';
			de = ' ';
		}

		sprintf(f, "%7.2f", tasks[i].deadline -
			(tasks[i].computation + I));
		if (!verbose)
			printf("%c%7s%c ", di, f, de);

		s += tasks[i].deadline - R;
	}

	if (!verbose) {
		printf("]\t%-4s ", ok ? "OK": "NOT");
		if (ok)
			printf("%7.2f", s);
		printf("\n");
	}
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

	if (verbose)
	/* O(ntasks) */
		print_task_influencies(ntasks, tasks);

	/* O(ntasks) */
	print_task_analysis(ntasks, tasks, verbose);
}
