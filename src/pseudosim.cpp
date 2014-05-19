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
static double deadline_min = 10, deadline_max = 50;
static double period_min = 10, period_max = 50;
static bool compare_no_lp = 0;

static double inline next_ak(double min, double max) {
	double v;

	v = AkRandomReal();
	v *= (max - min);
	v += min;

	return v;
}

static const char *short_options = "hvf:r:n:m:l:c";
static const struct option long_options[] = {
	{ "help",     0, NULL, 'h' },
	{ "verbose",  0, NULL, 'v' },
	{ "freq-file",  required_argument, NULL, 'f' },
	{ "range-file",  required_argument, NULL, 'r' },
	{ "task-count",  required_argument, NULL, 'n' },
	{ "switch-latency",  required_argument, NULL, 'l' },
	{ "processor-count",  required_argument, NULL, 'm' },
	{ "compare-no-lp",  0, NULL, 'c' },
	{ NULL,       0, NULL, 0   },   /* Required at end of array.  */
};

static void print_usage(char *program_name)
{
	printf("Usage: %s  options\n", program_name);
	printf(
	"  -h  --help                             Display this usage information.\n"
	"  -v  --verbose                          Print verbose messages.\n"
	"  -c  --compare-no-lp                    Compare the difference to test without A_i.\n"
	"  -f  --freq-file=<file-name>            File name with frequencies per cluster.\n"
	"  -r  --range-file=<file-name>           File name with task model ranges.\n"
	"  -n  --task-count=<task-count>          Number of tasks to be generated per task model.\n"
	"  -l  --switch-latency=<Lp>              Switching Latency.\n"
	"  -m  --processor-count=<freq-count>     Number of processors per cluster.\n");
}

static int read_ranges(char *range_file_name)
{
	ifstream file(range_file_name);

	file >> wcec_min >> wcec_max
		>> period_min >> period_max
		>> deadline_min >> deadline_max;

	return 0;
}

int compar(const void *a, const void *b) {
	double *la = (double *)a, *lb = (double *)b;

	/*we want decreasing order*/
	return *lb - *la;
}

static int read_frequencies(char *freq_file_name, IloNumArray2 &freqs, IloNumArray2 &volts)
{
	ifstream file(freq_file_name);

	file >> freqs >> volts;

	return 0;
}

/*
 * read_task_model: Reads needed info
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks, which will be filled up with task values
 * @parameter nresources: integer which represents the number of resources
 * @complexity: O(ntasks x nresources)
 */
static int gen_task_model(vector <class Task> &tasks, IloEnv env, int ntasks)
{
	int i = 0;

	for (i = 0; i < ntasks; i++) {

		tasks[i].setPriority(next_ak(1.0, ntasks));
		tasks[i].setPeriod(next_ak(period_min, period_max));
		tasks[i].setDeadline(next_ak(period_min, tasks[i].getPeriod()));
		tasks[i].setWcec(next_ak(wcec_min, wcec_max));
		tasks[i].setIp(0.0); /* do not touch for now */
		tasks[i].setIb(0.0);/* do not touch for now */
		tasks[i].setIa(0.0); /* do not touch for now */
		tasks[i].setIj(0.0); /* do not touch for now */

	}

	return 0;
}

static void gen_assignments(IloNumArray4 &assig, int nclusters, int nprocs, int ntasks, int nfreqs)
{
	int s, i, j, k;

	for (s = 0; s < nclusters; s++) {
		for (i = 0; i < nprocs; i++) {
			for (j = 0; j < ntasks; j++) {
				for (k = 0; k < nfreqs; k++) {
					assig[s][i][j][k] = 0;
				}
			}
		}
	}
	for (j = 0; j < ntasks; j++) {
		s = (int)floor(next_ak(0.0, (double)nclusters));
		i = (int)floor(next_ak(0.0, (double)nprocs));
		k = (int)floor(next_ak(0.0, (double)nfreqs));
		assig[s][i][j][k] = 1.0;
	}
}

static long get_execution_time(struct timeval s, struct timeval e)
{
	struct timeval diff;

	timersub(&e, &s, &diff);

	return diff.tv_sec * 1000000 + diff.tv_usec;
}

void leave(int sig) {
	/* clean up procedure */
}

/*
 * @complexity: O(nfrequencies ^ ntasks) x O(nresources x ntasks ^ 2)
 */
int main(int argc, char *argv[])
{
	struct timeval st, e;
	long times[6];
	IloEnv env;
	vector <class Task> tasks;
	IloNumArray2 freqs(env);
	IloNumArray2 volts(env);
	struct runInfo runtime;
	double Lp = 0.0;
	int nclusters = 0;
	int nprocs = 0;
	int ntasks = 0;
	int nfreqs = 0;
	int next_option;
	int err = 0;
	int s, i, j, k;
	char *freq_file_name = NULL;
	char *range_file_name = NULL;

	(void) signal(SIGTERM,leave);

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
			return 0;
		case 'v':   /* -v or --verbose */
			runtime.setVerbose(true);
			break;
		case 'c':   /* -c or --compare_no_lp */
			compare_no_lp = true;
			break;
		case 'l':   /* -l or --switch-latency */
			if (!optarg) {
				printf("Specify the frequency switching latency.\n");
				return -EINVAL;
			}
			Lp = strtod(optarg, NULL);
			break;
		case 'n':   /* -n or --task-count */
			if (!optarg) {
				printf("Specify the number of tasks.\n");
				return -EINVAL;
			}
			ntasks = strtol(optarg, NULL, 10);
			break;
		case 'm':   /* -m or --processor-count */
			if (!optarg) {
				printf("Specify the number of frequencies.\n");
				return -EINVAL;
			}
			nprocs = strtol(optarg, NULL, 10);
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

	err = read_frequencies(freq_file_name, freqs, volts);
	if (err < 0)
		return err;

	err = read_ranges(range_file_name);
	if (err < 0)
		return err;

	nclusters = freqs.getSize();
	nfreqs = freqs[0].getSize();

	if (nclusters == 0 || ntasks == 0 || nprocs == 0 || nfreqs == 0) {
		print_usage(argv[0]);
		return -EINVAL;
	}

	IloNumArray4 assig(env, nclusters);

	for (s = 0; s < nclusters; s++) {
		assig[s] = IloNumArray3(env, nprocs);
		for (i = 0; i < nprocs; i++) {
			assig[s][i] = IloNumArray2(env, ntasks);
			for (j = 0; j < ntasks; j++) {
				assig[s][i][j] = IloNumArray(env, nfreqs);
				for (k = 0; k < nfreqs; k++) {
					assig[s][i][j][k] = 0;
				}
				Task t(env);
				tasks.push_back(t);
			}
		}
	}


	if (compare_no_lp)
		AkDeclareParameters(8);
	else
		AkDeclareParameters(6);
	while (!AkSimulationOver()) {
		double sp, ui, n = ntasks;
		bool u_edf, u_ll, r;
		bool u_edf0, u_ll0, r0;

		/* O(nfrequencies) + O(ntasks x nresources) */
		err = gen_task_model(tasks, env, ntasks);
		if (err < 0) {
			printf("Error while generating task model\n");
			return err;
		}
		gen_assignments(assig, nclusters, nprocs, ntasks, nfreqs);
		SchedulabilityAnalysis sched(env, runtime, ntasks,
						0, /* nresources */
						Lp, /* Lp */
						freqs, volts, tasks, assig);
		gettimeofday(&st, NULL);
		sched.computeAnalysis();
		gettimeofday(&e, NULL);
		times[0] = get_execution_time(st, e);

		gettimeofday(&st, NULL);
		u_edf = sched.evaluateUtilization(1.0, ui);
		gettimeofday(&e, NULL);
		times[1] = get_execution_time(st, e);

		gettimeofday(&st, NULL);
		u_ll = sched.evaluateUtilization(n * (pow(2.0, 1.0 / n) - 1.0), ui);
		gettimeofday(&e, NULL);
		times[2] = get_execution_time(st, e);

		r = sched.evaluateResponse(sp);

		SchedulabilityAnalysis sched0(env, runtime, ntasks,
					0, /* nresources */
					0.0, /* Lp */
					freqs, volts, tasks, assig);

		gettimeofday(&st, NULL);
		sched0.computeAnalysis();
		gettimeofday(&e, NULL);
		times[3] = get_execution_time(st, e);

		gettimeofday(&st, NULL);
		u_edf0 = sched0.evaluateUtilization(1.0, ui);
		gettimeofday(&e, NULL);
		times[4] = get_execution_time(st, e);

		gettimeofday(&st, NULL);
		u_ll0 = sched0.evaluateUtilization(n * (pow(2.0, 1.0 / n) - 1.0), ui);
		gettimeofday(&e, NULL);
		times[5] = get_execution_time(st, e);

		r0 = sched0.evaluateResponse(sp);

		if (compare_no_lp) {
			AkParamObservation(1, r - u_edf0);
			AkParamObservation(2, r - u_ll0);
      			AkParamObservation(3, r - r0);
			AkParamObservation(4, ui);
			AkParamObservation(5, times[0]); /* R Lp */
			AkParamObservation(6, times[3]); /* R_0 */
			AkParamObservation(7, times[4]); /* u_edf0 */
			AkParamObservation(8, times[5]); /* u_ll0 */
		} else {
			AkParamObservation(1, r - u_edf);
			AkParamObservation(2, r - u_ll);
			AkParamObservation(3, ui);
			AkParamObservation(4, times[0]); /* R Lp */
			AkParamObservation(5, times[1]); /* u_edf */
			AkParamObservation(6, times[2]); /* u_ll */
		}
	}

	return err;
}
