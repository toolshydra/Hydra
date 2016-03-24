/*
 * src/smartenum.c
 *
 * Copyright (C) 2008 Eduardo Valentin <edubezval@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <signal.h>

#include <akaroa.H>
#include <akaroa/distributions.H>

#include <analysis.h>
#include <gcd_hash.h>

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

static double wcec_min = 100, wcec_max = 200;
static double deadline_min = 10, deadline_max = 50;
static double period_min = 10, period_max = 50;
static bool compare_no_lp = 0;
static bool compute_power = false;
static IloNumArray2 period_ranges;

/* shared memory */
/* reflects always the last thread execution */
struct thread_data {
	long etimes;
	bool good;
	double energyS;

	IloEnv *env;

	IloNumArray2 *freqs;
	IloNumArray2 *volts;
	IloNumArray2 *pdyn;
	IloNumArray2 *pidle;

	IloNumArray *priority;/*tasks*/
	IloNumArray *period;/*tasks*/
	IloNumArray *deadline;/*tasks*/
	IloNumArray2 *cycles;/*procs*/
};

static double inline next_ak(double min, double max)
{
	return Uniform(min, max);
}

static const char *short_options = "hvpa:u:f:s:r:n:l:m:c:g:o:";
static const struct option long_options[] = {
	{ "help",     0, NULL, 'h' },
	{ "verbose",  0, NULL, 'v' },
	{ "compute-power",  0, NULL, 'p' },
	{ "max-ui",  required_argument, NULL, 'a' },
	{ "u-total",  required_argument, NULL, 'u' },
	{ "freq-file",  required_argument, NULL, 'f' },
	{ "solvers-file",  required_argument, NULL, 's' },
	{ "range-file",  required_argument, NULL, 'r' },
	{ "task-count",  required_argument, NULL, 'n' },
	{ "model-count",  required_argument, NULL, 'g' },
	{ "output-location",  required_argument, NULL, 'o' },
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
	"  -p  --compute-power                    Estimate system average energy consumption.\n"
	"  -f  --freq-file=<file-name>            File name with frequencies per cluster.\n"
	"  -s  --solvers-file=<file-name>         File name with solvers command line.\n"
	"  -r  --range-file=<file-name>           File name with task model ranges.\n"
	"  -n  --task-count=<task-count>          Number of tasks to be generated per task model.\n"
	"  -g  --model-count=<model-count>        Number of models to be generated.\n"
	"  -o  --output-location=<dir>            Directory to output the generated models.\n"
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

static int read_period_ranges(char *range_file_name)
{
	ifstream file(range_file_name);

	file >> period_ranges;

	return 0;
}

int compar(const void *a, const void *b) {
	double *la = (double *)a, *lb = (double *)b;

	/*we want decreasing order*/
	return *lb - *la;
}

static int read_power_model(char *freq_file_name, IloNumArray2 &freqs, IloNumArray2 &pdyn,
				IloNumArray2 &pstat)
{
	ifstream file(freq_file_name);

	file >> freqs >> pdyn >> pstat;

	return 0;
}

static int read_frequencies(char *freq_file_name, IloNumArray2 &freqs, IloNumArray2 &volts)
{
	ifstream file(freq_file_name);

	file >> freqs >> volts;

	return 0;
}

static int read_solvers_config(char *solvers_file_name, int &nsolvers, int &nparams,
				vector <string> &solvers)
{
	ifstream file(solvers_file_name);
	string line;
	char str[2];
	int i;

	if (!solvers_file_name) {
		cerr << "You have to specify the solvers config file (-s)" <<
				endl;
		return -EINVAL;
	}

	if (!file.is_open()) {
		cerr << "Error reading solvers config file " <<
			solvers_file_name << endl;
		return -EINVAL;
	}

	nsolvers = 0;

	file >> nsolvers >> nparams;
	if (nsolvers <= 0 || nparams <= 0) {
		cerr << "Wrong solvers config file " <<
			solvers_file_name << endl;
		return -EINVAL;
	}

	solvers.clear();
	/* skip first with nsolvers nparams */
	std::getline(file, line);
	for (i = 0; i < nsolvers; i++) {

		if (file.eof()) {
			cerr << "Wrong solvers config file " <<
				solvers_file_name << endl;
			return -EINVAL;
		}

		std::getline(file, line);
		solvers.push_back(line);
	}
	file.close();

	return 0;
}

/*
 * read_task_model: Reads needed info
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks, which will be filled up with task values
 * @parameter nresources: integer which represents the number of resources
 * @complexity: O(ntasks x nresources)
 */
static int gen_utilization_model(double max_u, double max_ui, int nprocs,
				int max_freq, /*higher first, per proc, between procs*/
				IloNumArray &priority,
				IloNumArray &period,
				IloNumArray &deadline,
				IloNumArray2 &cycles)
{
	int i, j;
	double cycle_factor[nprocs], cur_u, min_ui = max_ui * 0.2;
	IloNumArray utilization = IloNumArray(priority.getEnv());
	IloDataCollection s;

	priority.clear();
	period.clear();
	deadline.clear();
	utilization.clear();
	for (j = 0; j < nprocs; j++) {
		cycles[j].clear();
		cycle_factor[j] = next_ak(0.8, 0.9);
	}
	cycle_factor[0] = 1.0;
	i = 0; cur_u = 0.0;
	while(cur_u < max_u) {
		double p_i, u_i, x, s;
		int range = UniformInt(0, period_ranges.getSize() - 1);

		/*
		 * Normally distributed between 10% and
		 * max_ui.
		 */
		x = (max_ui + min_ui) / 2.0;
		x *= 100.0;
		s = sqrt(max_ui * 100.0 - x);
		u_i = Normal(x, s);
		u_i /= 100.0;
		if ((u_i + cur_u) > max_u)
			u_i -= (u_i + cur_u) - max_u;
		if (u_i < min_ui)
			u_i = min_ui;
		p_i = Uniform(period_ranges[range][0],
				period_ranges[range][1]);
		utilization.add(u_i);
		deadline.add(p_i);
		period.add(p_i); /* period == deadline */

		cur_u += u_i;
		i++;
	}
	s.sort(deadline);
	s.sort(period);
	int n = i; i = 0;
	while (i < n) {
		double p_i, u_i, c_i, cycles_i;

		p_i = period[i];
		u_i = utilization[i];
		c_i = u_i * p_i; /* at max speed */
		cycles_i = c_i * max_freq;

		cycles[0].add(ceil(cycles_i));
		for (j = 1; j < nprocs; j++)
			cycles[j].add(ceil(cycles_i * cycle_factor[j]));

		priority.add(n - i);
		i++;
	}

	utilization.end();

	return 0;
}

/*
 * read_task_model: Reads needed info
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks, which will be filled up with task values
 * @parameter nresources: integer which represents the number of resources
 * @complexity: O(ntasks x nresources)
 */
static int gen_task_model(int ntasks, int nprocs, IloEnv env, IloNumArray &priority, IloNumArray &period,
			  IloNumArray &deadline, IloNumArray2 &cycles)
{
	int i, j;

	for (i = 0; i < ntasks; i++) {

		priority[i] = ntasks - i;
		deadline[i] = period[i] = next_ak(period_min, period_max);
		// deadline[i] = next_ak(period_min, period[i]);
		for (j = 0; j < nprocs; j++)
			cycles[j][i] = next_ak(wcec_min, wcec_max);
	}

	return 0;
}

void write_model(struct thread_data *tdata, string location)
{
	std::string result = "", filename = "";
	ofstream mfile;
	FILE* pipe;
	char buffer[128];

	pipe = popen((std::string("mktemp ") + location + "fileXXXXXXXX").c_str(), "r");

	if (!pipe)
		return;

	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			filename += buffer;
	}
	pclose(pipe);

	filename.erase(filename.size() - 1);

	mfile.open(filename.c_str());
	mfile << "1" << endl;
	mfile << *(tdata->priority) << endl;
	mfile << *(tdata->period) << endl;
	mfile << *(tdata->deadline) << endl;
	mfile << *(tdata->cycles) << endl;
	mfile << *(tdata->volts) << endl;
	mfile << *(tdata->freqs) << endl;
	mfile.close();
}

void leave(int sig) {
	/* clean up procedure */
}

/*
 * @complexity: O(nfrequencies ^ ntasks) x O(nresources x ntasks ^ 2)
 */
int main(int argc, char *argv[])
{
	struct thread_data *tdata;
	IloEnv env;
	struct runInfo runtime;
	double Lp = 0.0, max_ui = 0.0, u_total = 0.0;
	int nprocs = 0;
	int ntasks = 0;
	int nmodels = 0;
	int nfreqs = 0;
	int nsolvers = 0;
	int nparams = 0;
	int next_option;
	int err = 0;
	int s, i, j, k;
	char *freq_file_name = NULL;
	char *solvers_file_name = NULL;
	char *range_file_name = NULL;
	vector <string> solvers;
	vector <double> timeset;
	vector <double> energyset;
	string location;
	bool allgood;

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
		case 'p':   /* -p or --compute-power */
			compute_power = true;
			break;
		case 'a':   /* -a or --max-ui */
			if (!optarg) {
				printf("Specify the maximum utilization per task (<=1.0).\n");
				return -EINVAL;
			}
			max_ui = strtod(optarg, NULL);
			if (max_ui > 1.0) {
				printf("Specify the maximum utilization per task (<=1.0).\n");
				return -EINVAL;
			}
			break;
		case 'u':   /* -u or --u-total */
			if (!optarg) {
				printf("Specify the maximum system utilization (<=nprocs).\n");
				return -EINVAL;
			}
			u_total = strtod(optarg, NULL);
			break;
		case 'l':   /* -l or --switch-latency */
			if (!optarg) {
				printf("Specify the frequency switching latency.\n");
				return -EINVAL;
			}
			Lp = strtod(optarg, NULL);
			break;
		case 'o':   /* -o or --location */
			if (!optarg) {
				printf("Specify the location to output the models.\n");
				return -EINVAL;
			}
			location = string(optarg);
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
		case 'g':   /* -g or --model-count */
			if (!optarg) {
				printf("Specify the number of models.\n");
				return -EINVAL;
			}
			nmodels = strtol(optarg, NULL, 10);
			break;
		case 'f':   /* -f or --freq-file */
			if (!optarg) {
				fprintf(stderr, "Specify file with frequencies.\n");
				print_usage(argv[0]);
				return -EINVAL;
			}
			freq_file_name = optarg;
			break;
		case 's':   /* -s or --solver-file */
			if (!optarg) {
				fprintf(stderr, "Specify file with solvers configs.\n");
				print_usage(argv[0]);
				return -EINVAL;
			}
			solvers_file_name = optarg;
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

	err = read_solvers_config(solvers_file_name, nsolvers,
					nparams, solvers);
	if (err < 0)
		return err;

	tdata = new (struct thread_data);
	tdata->env = &env;

	tdata->freqs = new IloNumArray2(env);
	tdata->volts = new IloNumArray2(env);
	tdata->pdyn = new IloNumArray2(env);
	tdata->pidle = new IloNumArray2(env);

	if (compute_power)
		err = read_power_model(freq_file_name, *(tdata->freqs), *(tdata->pdyn), *(tdata->pidle));
	else
		err = read_frequencies(freq_file_name, *(tdata->freqs), *(tdata->volts));

	if (err < 0)
		return err;

	if (u_total > 0) {
		period_ranges = IloNumArray2(env);
		err = read_period_ranges(range_file_name);
	} else {
		err = read_ranges(range_file_name);
	}
	if (err < 0)
		return err;

	nprocs = tdata->freqs->getSize();
	nfreqs = (*tdata->freqs)[0].getSize();

	if (ntasks == 0 || nprocs == 0 || nfreqs == 0) {
		print_usage(argv[0]);
		return -EINVAL;
	}

	if (u_total > nprocs) {
		printf("Specify the maximum system utilization (<=nprocs).\n");
		return -EINVAL;
	}

	tdata->priority = new IloNumArray(env, ntasks);
	tdata->period = new IloNumArray(env, ntasks);
	tdata->deadline = new IloNumArray(env, ntasks);
	tdata->cycles = new IloNumArray2(env, nprocs);

	for (int j = 0; j < nprocs; j++)
		tdata->cycles[0][j] = IloNumArray(env, ntasks);

	/*
	 * For now we are assuming a fixed number of parameters
	 * per solver:
	 * 1. Feasibility: Always sent to akaroa2. Represents the
	 *		   ratio that the test can find feasible
	 *		   solutions for a given simulation setup.
	 * 2. Execution time feasible: Sent to akaroa2 only if the
	 *		   test succeed to find a feasible solution.
	 *		   Represents how long the test spends
	 *		   attempting to find  the optimal a configuration
	 *		   in a simulation setup that has feasible solution.
	 * 3. Energy feasible: Sent to akaroa2 only if the test
	 *		       finds a feasible solution and all other
	 *		       tests as well. Represents
	 *		       the minimal energy the test can find
	 *		       for a given simulation setup.
	 */

	i = 0;
	while (i++ < nmodels) {

		/* O(nfrequencies) + O(ntasks x nresources) */
		if (u_total > 0)
			err = gen_utilization_model(u_total, max_ui,
					nprocs, (*tdata->freqs)[0][0],
					*(tdata->priority),
					*(tdata->period), *(tdata->deadline),
					*(tdata->cycles));
		else
			err = gen_task_model(ntasks, nprocs, env, *(tdata->priority),
					*(tdata->period), *(tdata->deadline),
					*(tdata->cycles));
		write_model(tdata, location);
	}

	return err;
}
