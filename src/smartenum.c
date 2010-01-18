/*
 * src/smartenum.c
 *
 * Copyright (C) 2008 Eduardo Valentin <edubezval@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include <analysis.h>

static const char *short_options = "hvstiljk";
static const struct option long_options[] = {
	{ "help",     0, NULL, 'h' },
	{ "verbose",  0, NULL, 'v' },
	{ "summary",  0, NULL, 's' },
	{ "tabular",  0, NULL, 't' },
	{ "best-initial-limits",  0, NULL, 'i' },
	{ "list-samples",  0, NULL, 'l' },
	{ "jump-useless",  0, NULL, 'j' },
	{ "start-drop",  0, NULL, 'k' },
	{ "heuristic",  required_argument, NULL, 1 },
	{ NULL,       0, NULL, 0   },   /* Required at end of array.  */
};

static void print_usage(char *program_name)
{
	printf("Usage:  %s options\n", program_name);
	printf(
	"  -h  --help                        Display this usage information.\n"
	"  -v  --verbose                     Print verbose messages.\n"
	"  -s  --summary                     Print overall total numbers.\n"
	"  -t  --tabular                     Print overall total numbers in one line.\n"
	"       (Total samples | Evaluated samples | Feasible samples | Time of processing).\n"
	"  -i  --best-initial-limits         Removes combinations by "
					"limiting too low frequencies.\n"
	"  -l  --list-samples                List each sample summary"
						" analysis.\n"
	"  -j  --jump-useless                Jump useless detected samples.\n"
	"  -k  --start-drop                  Drop initial useless samples.\n"
	"      --heuristic={usage_first}     Specify an heuristic to use.\n");
}
/*
 * read_array: Reads an array of floats which may represent a sequence of
 * 		task's properties
 * @parameter n: number of floats
 * @parameter a: array of floats, which will be filled with read values
 * @complexity: O(n)
 */
static int read_array(int n, double **a)
{
	int j = 0;
	double *c;


	if (!*a) {
		printf("Could not allocate memory for array\n");
		return -ENOMEM;
	}

	c = *a;

	do {
		scanf("%lf", &c[j]);
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
static int read_task_model(struct task_set *tset, struct freq_set *freqs,
				int nresources)
{
	int i = 0;
	int err;


	if (!tset->tasks) {
		printf("Could not allocate memory for %d tasks.\n",
							tset->ntasks);
		return -ENOMEM;
	}

	/* O(nfrequencies) */
	err = read_array(freqs->nfrequencies, &freqs->frequencies);
	if (err < 0) {
		printf("Could not read array of frequencies.\n");
		return err;
	}

	err = read_array(freqs->nfrequencies, &freqs->voltages);
	if (err < 0) {
		printf("Could not read array of frequencies.\n");
		return err;
	}

	do {
		struct task *t = tset->tasks + i;

		scanf("%lf %lf %lf", &t->wcec, &t->deadline, &t->Ij);

		err = read_array(nresources, &t->resources); /* O(nres) */
		if (err < 0) {
			printf("Could not read array of resources for task"
				" %d.\n", i);
			return err;
		}

	} while (++i < tset->ntasks);

	return 0;
}

/*
 * print_summary: print statistics about the execution
 * @parameter tset: set of tasks
 * @parameter freqs: set of tasks
 * @parameter stat: information about the execution
 * @complexity: O(ntasks)
 */
void print_summary(struct task_set tset, struct freq_set freqs,
			struct results stat)
{
	int i;
	struct timeval diff;

	printf("Sumário\n");
	printf("Número de Configurações: %6.0lf\n",
				pow(freqs.nfrequencies, tset.ntasks));
	printf("Configurações Avaliadas: %6d\n", stat.total);
	printf("Configurações Viáveis: %d\n", stat.success);

	timersub(&stat.e, &stat.s, &diff);
	printf("Tempo de processamento: %lds and %ld us\n", diff.tv_sec,
						diff.tv_usec);
	if (stat.best < HUGE_VAL) {
		double sys_utilization = 0;
		double energy_a = 0;
		double energy_b = 0;

		printf("Melhor espalhamento %.2lf com as seguintes frequências\n",
			stat.best);
		for (i = 0; i < tset.ntasks; i++) {
			double frequency, voltage_a, voltage_b;
			double utilization;

			frequency = freqs.frequencies[stat.best_index[i]];
			voltage_a = freqs.voltages[stat.best_index[i]];
			voltage_b = freqs.voltages[0];
			printf("(%.2lf; %.2lf) ", frequency, voltage_a);
			utilization = (tset.tasks[i].wcec / frequency);
			utilization /= tset.tasks[i].deadline;

			sys_utilization += utilization;

			energy_a += tset.tasks[i].wcec * (voltage_a * voltage_a);
			energy_b += tset.tasks[i].wcec * (voltage_b * voltage_b);
		}

		printf("\nUtilização total do sistema é %6.2lf%\n",
			sys_utilization * 100);
		printf("Energia gasta pelo sistema é %6.2lf x C\n",
			energy_a);
		printf("Energia gasta pelo sistema é %6.2lf x C "
			"se usar apenas a maior frequência\n",
			energy_b);
		printf("Redução de energia: %6.2lf%\n",
			((energy_b - energy_a) / energy_b) * 100);
	}
}

/*
 * print_tabular: print statistics about the execution in one line
 * @parameter tset: set of tasks
 * @parameter freqs: set of tasks
 * @parameter stat: information about the execution
 * @complexity: O(1)
 */
void print_tabular(struct task_set tset, struct freq_set freqs,
			struct results stat)
{
	int i;
	struct timeval diff;

	printf("%6.0lf\t", pow(freqs.nfrequencies, tset.ntasks));
	printf("%6d\t", stat.total);
	printf("%d\t", stat.success);

	timersub(&stat.e, &stat.s, &diff);
	printf("%ld.%06ld\n", diff.tv_sec, diff.tv_usec);
}

/*
 * @complexity: O(nfrequencies ^ ntasks) x O(nresources x ntasks ^ 2)
 */
int main(int argc, char *argv[])
{
	struct task_set tset;
	struct res_set res;
	struct freq_set freqs;
	struct run_info runtime;
	struct results stat;
	int *limits;
	int next_option;
	int heuristic = 0;
	int err = 0;

	memset(&runtime, 0, sizeof(runtime));

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
			runtime.verbose = 1;
			break;
		case 's':   /* -s or --summary */
			runtime.summary = 1;
			break;
		case 't':   /* -t or --tabular */
			runtime.tabular = 1;
			break;
		case 'i':   /* -i or --best-initial-limits */
			runtime.best_initial_limits = 1;
			break;
		case 'l':   /* -l or --list-samples */
			runtime.list = 1;
			break;
		case 'j':   /* -j or --jump-useless */
			runtime.jump_samples = 1;
			break;
		case 'k':   /* -s or --start-drop */
			runtime.best_start = 1;
			break;
		case 1:   /* -e or --heuristic */
			if (!optarg) {
				printf("Specify an heuristic.\n");
				return -EINVAL;
			}
			if (!strcmp(optarg, "usage_first"))
				heuristic = 1;
			if (!heuristic) {
				printf("Available heuristics: usage_first\n");
				return -EINVAL;
			}
			break;
		case -1:    /* Done with options.  */
			break;
		}
	} while (next_option != -1);

	/* Read input data */
	scanf("%d %d %d", &tset.ntasks, &freqs.nfrequencies, &res.nresources);

	err = enumeration_init(&tset, &freqs, &stat, &res, &limits);
	if (err < 0)
		goto exit;

	/* O(nfrequencies) + O(ntasks x nresources) */
	err = read_task_model(&tset, &freqs, res.nresources);
	if (err < 0) {
		printf("Error while reading task model\n");
		goto exit;
	}

	/* Compute output data */
	gettimeofday(&stat.s, NULL);
	err = compute_initial_limits(tset, freqs, runtime, &limits);
	if (err < 0) {
		printf("Error while computing initial limits\n");
		goto exit;
	}

	err = enumerate_samples(tset, freqs, &res, limits, runtime, &stat);
	if (err < 0) {
		printf("Error while enumerating samples\n");
		goto exit;
	}

	gettimeofday(&stat.e, NULL);

	if (runtime.summary)
		print_summary(tset, freqs, stat);

	if (runtime.tabular)
		print_tabular(tset, freqs, stat);

	/* clean up procedure */
	enumeration_cleanup(&tset, &freqs, &stat, &res, limits);
exit:
	return err;
}
