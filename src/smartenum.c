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
#include <getopt.h>

#include <analysis.h>

static const char *short_options = "hvsiljk";
static const struct option long_options[] = {
	{ "help",     0, NULL, 'h' },
	{ "verbose",  0, NULL, 'v' },
	{ "summary",  0, NULL, 's' },
	{ "initial-drop",  0, NULL, 'i' },
	{ "list-samples",  0, NULL, 'l' },
	{ "jump-useless",  0, NULL, 'j' },
	{ "start-drop",  0, NULL, 'k' },
	{ NULL,       0, NULL, 0   },   /* Required at end of array.  */
};

static void print_usage(char *program_name)
{
	printf("Usage:  %s options\n", program_name);
	printf(
		"  -h  --help             Display this usage information.\n"
		"  -v  --verbose          Print verbose messages.\n"
		"  -s  --summary          Print overall total numbers.\n"
		"  -i  --initial-drop     Removes combinations by limiting too "
							"low frequencies.\n"
		"  -l  --list-samples     List each sample summary"
							" analysis.\n"
		"  -j  --jump-useless     Jump useless detected samples.\n"
		"  -k  --start-drop       Drop initial useless samples.\n");
}

/*
 * read_array: Reads an array of floats which may represent a sequence of
 * 		task's properties
 * @parameter n: number of floats
 * @parameter a: array of floats, which will be filled with read values
 * @complexity: O(n)
 */
static int read_array(int n, float **a)
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
static int read_task_model(int ntasks, struct task **tasks, int nfrequencies,
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
	time_t s, e;
	int ntasks;
	int nresources;
	int nfrequencies;
	float *frequencies;
	int *resource_priorities;
	struct task *tasks;
	int success;
	int total;
	int *limits;
	int next_option;
	int verbose = 0;
	int summary = 0;
	int drop = 0;
	int list_samples = 0;
	int jump_useless = 0;
	int start_drop = 0;
	int err = 0;

	/* Read command line options */
	do {
		next_option = getopt_long (argc, argv, short_options,
						long_options, NULL);
		switch (next_option) {
		default:    /* Something else: unexpected.  */
		case '?':   /* The user specified an invalid option.  */
			err = -EINVAL;
		case 'h':   /* -h or --help */
			print_usage(argv[0]);
			goto exit;
		case 'v':   /* -v or --verbose */
			verbose = 1;
			break;
		case 's':   /* -s or --summary */
			summary = 1;
			break;
		case 'i':   /* -i or --initial-drop */
			drop = 1;
			break;
		case 'l':   /* -l or --list-samples */
			list_samples = 1;
			break;
		case 'j':   /* -j or --jump-useless */
			jump_useless = 1;
			break;
		case 'k':   /* -s or --start-drop */
			start_drop = 1;
			break;
		case -1:    /* Done with options.  */
			break;
		}
	} while (next_option != -1);

	/* Read input data */
	scanf("%d %d %d", &ntasks, &nfrequencies, &nresources);

	/* O(nfrequencies) + O(ntasks x nresources) */
	err = read_task_model(ntasks, &tasks, nfrequencies, &frequencies,
							nresources);
	if (err < 0) {
		printf("Error while reading task model\n");
		goto exit;
	}

	/* Compute output data */
	time(&s);
	err = compute_initial_limits(ntasks, tasks, nfrequencies, frequencies,
							drop, &limits);
	if (err < 0) {
		printf("Error while computing initial limits\n");
		goto exit;
	}

	err = enumerate_samples(ntasks, tasks, nfrequencies, frequencies,
				nresources, resource_priorities, limits,
				verbose, list_samples, start_drop,
				jump_useless, &success,	&total);
	if (err < 0) {
		printf("Error while enumerating samples\n");
		goto exit;
	}

	time(&e);

	if (summary) {
		printf("Summary\n");
		printf("Number of Samples: %6.0f\n",
					pow(nfrequencies, ntasks));
		printf("Number of Evaluated Samples: %6d\n", total);
		printf("Number of Feasible Samples: %d\n", success);
		printf("Time of processing: %.2fs\n", difftime(e, s));
	}

exit:
	return err;
}
