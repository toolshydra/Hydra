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
#include <signal.h>

#include <akaroa.H>

#include <analysis.h>

static double wcec_min = 100, wcec_max = 200;
static double period_min = 10, period_max = 50;
static double usage_min = 0, usage_max = .25;

static double inline next_ak(double min, double max) {
	double v;

	v = AkRandomReal();
	v *= (max - min);
	v += min;

	return v;
}

static const char *short_options = "hvf:tiljkn:m:p:r:";
static const struct option long_options[] = {
	{ "help",     0, NULL, 'h' },
	{ "verbose",  0, NULL, 'v' },
	{ "freq-file",  required_argument, NULL, 'f' },
	{ "range-file",  required_argument, NULL, 'p' },
	{ "tabular",  0, NULL, 't' },
	{ "best-initial-limits",  0, NULL, 'i' },
	{ "list-samples",  0, NULL, 'l' },
	{ "jump-useless",  0, NULL, 'j' },
	{ "start-drop",  0, NULL, 'k' },
	{ "task-count",  required_argument, NULL, 'n' },
	{ "freq-count",  required_argument, NULL, 'm' },
	{ "res-count",  required_argument, NULL, 'p' },
	{ NULL,       0, NULL, 0   },   /* Required at end of array.  */
};

static void print_usage(char *program_name)
{
	printf("Usage: %s -m nfreq -f <file> options\n", program_name);
	printf(
	"  -h  --help                        Display this usage information.\n"
	"  -v  --verbose                     Print verbose messages.\n"
	"  -f  --freq-file=<file-name>       File name with frequencies.\n"
	"  -r  --range-file=<file-name>      File name with wcec range.\n"
	"  -t  --tabular                     Print overall total numbers in one line.\n"
	"       (Total samples | Evaluated samples | Feasible samples | Time of processing).\n"
	"  -i  --best-initial-limits         Removes combinations by "
					"limiting too low frequencies.\n"
	"  -l  --list-samples                List each sample summary"
						" analysis.\n"
	"  -j  --jump-useless                Jump useless detected samples.\n"
	"  -k  --start-drop                  Drop initial useless samples.\n"
	"  -n  --task-count=<res-count>      Number of frequencies.\n"
	"  -m  --freq-count=<freq-count>     Number of frequencies.\n"
	"  -p  --res-count=<res-count>       Number of resources.\n");
}

static int read_ranges(char *range_file_name)
{
	int err = 0;
	FILE *f;

	f = fopen(range_file_name, "r");
	if (!f) {
		printf("Could not open range file %s\n",
							range_file_name);
		return -EIO;
	}

	fscanf(f, "%lf %lf %lf %lf %lf %lf",
		&wcec_min, &wcec_max,
		&period_min, &period_max,
		&usage_min, &usage_max);
	fclose(f);

	return err;
}

/*
 * read_array: Reads an array of floats which may represent a sequence of
 * 		task's properties
 * @parameter n: number of floats
 * @parameter a: array of floats, which will be filled with read values
 * @complexity: O(n)
 */
static int read_array(FILE *f, int n, double **a)
{
	int j = 0;
	double *c;

	c = *a;

	do {
		fscanf(f, "%lf", &c[j]);
	} while (++j < n);

	return 0;

}

int compar(const void *a, const void *b) {
	double *la = (double *)a, *lb = (double *)b;

	/*we want decreasing order*/
	return *lb - *la;
}

static int read_frequencies(char *freq_file_name, struct freq_set *freqs)
{
	int err = 0;
	FILE *f;

	f = fopen(freq_file_name, "r");
	if (!f) {
		printf("Could not open freq file %s\n",
							freq_file_name);
		return -EIO;
	}

	/* O(nfrequencies) */
	err = read_array(f, freqs->nfrequencies, &freqs->frequencies);
	if (err < 0) {
		printf("Could not read array of frequencies.\n");
		return err;
	}

	qsort(freqs->frequencies, freqs->nfrequencies, sizeof(double), compar);

	/* O(nfrequencies) */
	err = read_array(f, freqs->nfrequencies, &freqs->voltages);
	if (err < 0) {
		printf("Could not read array of voltages.\n");
		return err;
	}
	qsort(freqs->voltages, freqs->nfrequencies, sizeof(double), compar);

	fclose(f);

	return err;
}

/*
 * gen_array: Generates an array of doubles which random number between min and max
 * @parameter n: number of doubles
 * @parameter a: array of doubles, which will be filled with read values
 * @complexity: O(n)
 */
static int gen_array(int n, double **a, double min, double max)
{
	int j = 0;
	double *c;

	c = *a;

	do {
		c[j] = next_ak(min, max);
	} while (++j < n);

	return 0;

}

/*
 * read_task_model: Reads needed info
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks, which will be filled up with task values
 * @parameter nresources: integer which represents the number of resources
 * @complexity: O(ntasks x nresources)
 */
static int gen_task_model(struct task_set *tset, int nresources)
{
	int i = 0;
	int err;
	FILE *f;

	do {
		struct task *t = tset->tasks + i;

		t->wcec = next_ak(wcec_min, wcec_max);
		t->deadline = next_ak(period_min, period_max);
		t->Ij = 1;

		err = gen_array(nresources, &t->resources, usage_min,
								usage_max);
		if (err < 0) {
			printf("Could not read array of resources for task"
				" %d.\n", i);
			return err;
		}

	} while (++i < tset->ntasks);

	return 0;
}

static long get_execution_time(struct results stat)
{
	struct timeval diff;

	timersub(&stat.e, &stat.s, &diff);

	return diff.tv_sec * 1000000 + diff.tv_usec;
}

/*
 * print_tabular: print statistics about the execution in one line
 * @parameter tset: set of tasks
 * @parameter freqs: set of tasks
 * @parameter stat: information about the execution
 * @complexity: O(1)
 */
static void print_tabular(struct task_set tset, struct freq_set freqs,
			struct results stat)
{
	int i;
	struct timeval diff;

	printf("%6.0f\t", pow(freqs.nfrequencies, tset.ntasks));
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
	int err = 0;
	int i;
	char *freq_file_name = NULL;
	char *range_file_name = NULL;

void leave(int sig) {
	/* clean up procedure */
	enumeration_cleanup(&tset, &freqs, &stat, &res, limits);
}

	(void) signal(SIGTERM,leave);

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
		case 'n':   /* -n or --task-count */
			if (!optarg) {
				printf("Specify the number of tasks.\n");
				return -EINVAL;
			}
			tset.ntasks = strtol(optarg, NULL, 10);
			break;
		case 'm':   /* -m or --freq-count */
			if (!optarg) {
				printf("Specify the number of frequencies.\n");
				return -EINVAL;
			}
			freqs.nfrequencies = strtol(optarg, NULL, 10);
			break;
		case 'p':   /* -p or --res-count */
			if (!optarg) {
				printf("Specify the number of resources.\n");
				return -EINVAL;
			}
			res.nresources = strtol(optarg, NULL, 10);
			break;
		case 'f':   /* -f or --freq-file */
			if (!optarg) {
				fprintf(stderr, "Specify file with frequencies.\n");
				print_usage(argv[0]);
				return -EINVAL;
			}
			freq_file_name = optarg;
			break;
		case 'r':   /* -r or --range-file */
			if (!optarg) {
				fprintf(stderr, "Specify file with ranges.\n");
				print_usage(argv[0]);
				return -EINVAL;
			}
			range_file_name = optarg;
			break;
		case -1:    /* Done with options.  */
			break;
		}
	} while (next_option != -1);

	err = enumeration_init(&tset, &freqs, &stat, &res, &limits);
	if (err < 0)
		goto exit;

	err = read_frequencies(freq_file_name, &freqs);
	if (err < 0)
		goto exit;

	err = read_ranges(range_file_name);
	if (err < 0)
		goto exit;

	/* evaluated and time */
	AkDeclareParameters(4);
	while (!AkSimulationOver()) {
		double energy_b = 0;
		double energy_a = 0;

		/* O(nfrequencies) + O(ntasks x nresources) */
		err = gen_task_model(&tset, res.nresources);
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

		if (runtime.tabular)
			print_tabular(tset, freqs, stat);

		AkParamObservation(1,
				stat.total / pow(freqs.nfrequencies, tset.ntasks));
		AkParamObservation(2,
				stat.success / pow(freqs.nfrequencies, tset.ntasks));
		AkParamObservation(3, get_execution_time(stat));

		for (i = 0; i < tset.ntasks; i++) {
			double voltage_a, voltage_b;

			voltage_a = freqs.voltages[stat.best_index[i]];
			voltage_b = freqs.voltages[0];

			energy_a += tset.tasks[i].wcec * (voltage_a * voltage_a);
			energy_b += tset.tasks[i].wcec * (voltage_b * voltage_b);
		}

		if (stat.success > 0)
			AkParamObservation(4, energy_a / energy_b);

	}

exit:
	return err;
}
