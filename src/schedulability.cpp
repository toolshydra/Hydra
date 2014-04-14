/*
 * src/schedulability.cpp
 *
 * Copyright (C) 2008-2014 Eduardo Valentin <edubezval@gmail.com>
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

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include <analysis.h>

static const char *short_options = "hvstlm:";
static const struct option long_options[] = {
	{ "help",     0, NULL, 'h' },
	{ "verbose",  0, NULL, 'v' },
	{ "summary",  0, NULL, 's' },
	{ "list-samples",  0, NULL, 'l' },
	{ "model-file",  required_argument, NULL, 'm' },
	{ NULL,       0, NULL, 0   },   /* Required at end of array.  */
};

static void print_usage(char *program_name)
{
	cout << "Usage:  " << program_name << " options" << endl;
	cout <<
	"  -h  --help                        Display this usage information." << endl<< 
	"  -v  --verbose                     Print verbose messages." << endl <<
	"  -s  --summary                     Print overall total numbers." << endl <<
	"  -l  --list-samples                list each sample summary analysis." << endl <<
	"  -m  --model-file                  Specify where to read the model." << endl;
}

/*
 * print_summary: print statistics about the execution
 * @parameter tset: set of tasks
 * @parameter freqs: set of tasks
 * @parameter stat: information about the execution
 * @complexity: O(ntasks)
 */
static void print_summary(SchedulabilityAnalysis sched)
{
#if 0
	int i;
	struct timeval diff;

	printf("Summary\n");
	printf("Number of Configurations: %6.0lf\n",
				pow(freqs.nfrequencies, tset.ntasks));
	printf("Evaluated configurations: %6d\n", stat.total);
	printf("Feasible configurations: %d\n", stat.success);

	timersub(&stat.e, &stat.s, &diff);
	printf("Processing time: %lds and %d us\n", diff.tv_sec,
						diff.tv_usec);
	if (stat.best < HUGE_VAL) {
		double sys_utilization = 0;
		double energy_a = 0;
		double energy_b = 0;

		printf("Best spreading %.2lf with the following frequencies\n",
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

		printf("\nSystem's total utilization is %6.2lf%%\n",
			sys_utilization * 100);
		printf("System's required energy is %6.2lf x C\n",
			energy_a);
		printf("The energy used by the system is %6.2lf x C "
			"when only the highest frequency is used\n",
			energy_b);
		printf("Energy consumption reduction: %6.2lf%%\n",
			((energy_b - energy_a) / energy_b) * 100);
	}
#endif
}

/*
 * @complexity: O(nfrequencies ^ ntasks) x O(nresources x ntasks ^ 2)
 */
int main(int argc, char *argv[])
{
	IloEnv env;
	const char *filename = "model.txt";
	runInfo runtime;
	int next_option;
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
			return 0;
		case 'v':   /* -v or --verbose */
			runtime.setVerbose(true);
			break;
		case 's':   /* -s or --summary */
			runtime.setSummary(true);
			break;
		case 'l':   /* -l or --list-samples */
			runtime.setList(true);
			break;
		case 'm':   /* -m or --model-file */
			if (!optarg) {
				cerr << "Specify filename" << endl;
				return -EINVAL;
			}
			filename = optarg;
		case -1:    /* Done with options.  */
			break;
		}
	} while (next_option != -1);

	SchedulabilityAnalysis sched(env, runtime, filename);
	sched.computeAnalysis();

	/* Compute output data */

	if (runtime.getSummary())
		print_summary(sched);

	return err;
}
