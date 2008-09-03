/*
 * src/smartenum.c
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
 * read_array: Reads an array of floats which may represent a sequence of
 * 		task's properties
 * @parameter n: number of floats
 * @parameter a: array of floats, which will be filled with read values
 * @complexity: O(n)
 */
int read_array(int n, float **a)
{
	int j = 0;
	float *c;

	*a = malloc(sizeof(float) * n);

	if (!a) {
		printf("Could not allocate memory for array\n");
		return -ENOMEM;
	}

	c = *a;

	do {
		scanf("%f", &c[j]);
	} while (++j < n);

	return 0;

}

/*
 * read_task_model: Reads needed info
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks, which will be filled up with task values
 * @parameter nfrequencies: number of frequencies
 * @parameter frequencies: array of floats which will be filled up with
 * 			   available frequencies
 * @parameter nresources: integer which represents the number of resources
 * @complexity: O(nfrequencies) + O(ntasks x nresources)
 */
int read_task_model(int ntasks, struct task **tasks, int nfrequencies,
			float **frequencies, int nresources)
{
	int i = 0;
	int err;

	*tasks = malloc(ntasks * sizeof(**tasks));

	if (!*tasks) {
		printf("Could not allocate memory for %d tasks.\n", ntasks);
		return -ENOMEM;
	}

	err = read_array(nfrequencies, frequencies); /* O(nfrequencies) */
	if (err < 0) {
		printf("Could not read array of frequencies.\n");
		return err;
	}

	do {
		struct task *t = *tasks + i;

		scanf("%f %f %f", &t->wcec, &t->deadline, &t->Ij);

		err = read_array(nresources, &t->resources); /* O(nres) */
		if (err < 0) {
			printf("Could not read array of resources for task"
				" %d.\n", i);
			return err;
		}


	} while (++i < ntasks);

	return 0;
}

/*
 * @complexity: O(nfrequencies ^ ntasks) x O(nresources x ntasks ^ 2)
 */
int main(int argc, char *argv[])
{
	int ntasks;
	int nresources;
	int nfrequencies;
	float *frequencies;
	int *resource_priorities;
	struct task *tasks;
	int verbose = 0;

	int i, j;
	int *limits;
	int *ind;

	if (argc > 1)
		verbose = strcmp(argv[1], "-v") == 0;

	if (verbose) {
		printf("*-------------------------------------------------*\n");
		printf("*Scalability Test and Initial Frequency Calculator*\n");
		printf("*-------------------------------------------------*\n");
	}

	scanf("%d %d %d", &ntasks, &nfrequencies, &nresources);

	/* O(nfrequencies) + O(ntasks x nresources) */
	if (read_task_model(ntasks, &tasks, nfrequencies, &frequencies,
							nresources) < 0) {
		printf("Error while reading task model\n");
		return -EINVAL;
	}


	ind = malloc(ntasks * sizeof(int));
	if (!ind) {
		printf("Could not allocate memory for indices.\n");
		return -ENOMEM;
	}
	memset(ind, ntasks, 0);

	limits = malloc(ntasks * sizeof(int));
	if (!limits) {
		printf("Could not allocate memory for indices.\n");
		return -ENOMEM;
	}
	for (i = 0 ; i < ntasks; i++)
		limits[i] = nfrequencies;

	for (i = nfrequencies - 1; i >= 0 ; i--) {
		for (j = 0; j < ntasks; j++) {
			tasks[j].computation = tasks[j].wcec / frequencies[i];
			if (tasks[j].computation > tasks[j].deadline)
				limits[j] = i;
		}
	}

	j = 1;
	while (ind[0] < limits[0]) {

		printf("%03d -", j++);
		for (i = 0; i < ntasks; i++) {
			tasks[i].computation = tasks[i].wcec /
							frequencies[ind[i]];
		}

		compute_sample_analysis(ntasks, tasks, nresources, verbose);
		evaluate_sample_response(ntasks, tasks);

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
