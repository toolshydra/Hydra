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
#include <math.h>
#include <float.h>

#include <analysis.h>
#include <gcd_hash.h>

#include <string>

/* Constructors */
SchedulabilityAnalysis::SchedulabilityAnalysis(IloEnv &env, runInfo runtime)
	:tasks(_tasks), frequencies(_frequencies), voltages(_voltages),
	pdyn(_pdyn), pidle(_pidle), assignment(_assignment),
	resourcePriorities(_resourcePriorities),
	nClusters(0), nProcessors(0), nTasks(0), nFrequencies(0), nResources(0),
	Lp(0.0), runConfig(runtime), fileModel("model.txt"), loaded(false)
{
	runConfig = runtime;
	if (runConfig.getVerbose())
		cout << runConfig;

	readModel(env);
	distributeTaskFrequencies();
}

SchedulabilityAnalysis::SchedulabilityAnalysis(IloEnv &env, runInfo runtime, const char *filename,
						bool useAssignment)
	:tasks(_tasks), frequencies(_frequencies), voltages(_voltages),
	pdyn(_pdyn), pidle(_pidle), assignment(_assignment),
	resourcePriorities(_resourcePriorities),
	nClusters(0), nProcessors(0), nTasks(0), nFrequencies(0), nResources(0),
	Lp(0.0), runConfig(runtime), fileModel(filename), loaded(false)
{

	if (runConfig.getVerbose())
		cout << runConfig;

	readModel(env);

	if (useAssignment)
		distributeTaskFrequencies();

}

SchedulabilityAnalysis::SchedulabilityAnalysis(IloEnv &env, runInfo runtime, int ntask,
		int nresources, double lp, IloNumArray2 &freqs, IloNumArray2 &volts,
		vector <class Task> &tset, IloNumArray4 &assig)
	:frequencies(freqs), voltages(volts),
	pdyn(_pdyn), pidle(_pidle), assignment(assig),
	resourcePriorities(_resourcePriorities),
	nClusters(assig.getSize()), nProcessors(assig[0].getSize()),
	nTasks(assig[0][0].getSize()), nFrequencies(assig[0][0][0].getSize()),
	nResources(nresources), tasks(tset), Lp(lp), runConfig(runtime), fileModel(""), loaded(true)
{
	int i;

	if (runConfig.getVerbose())
		cout << runConfig;

	for (i = 0; i < nresources; i++)
		resourcePriorities.add(-1.0);

	assert(ntask != nTask);
	distributeTaskFrequencies();

	/* TODO: computePower() */
}

SchedulabilityAnalysis::SchedulabilityAnalysis(IloEnv &env, runInfo runtime, int ntask,
		int nresources, double lp, IloNumArray2 &freqs, IloNumArray2 &power_dyn,
		IloNumArray2 &power_idle, vector <class Task> &tset, IloNumArray4 &assig)
	:frequencies(freqs), voltages(_voltages),
	pdyn(power_dyn), pidle(power_idle), assignment(assig),
	resourcePriorities(_resourcePriorities),
	nClusters(assig.getSize()), nProcessors(assig[0].getSize()),
	nTasks(assig[0][0].getSize()), nFrequencies(assig[0][0][0].getSize()),
	nResources(nresources), tasks(tset), Lp(lp), runConfig(runtime), fileModel(""), loaded(true)
{
	int i;

	if (runConfig.getVerbose())
		cout << runConfig;

	for (i = 0; i < nresources; i++)
		resourcePriorities.add(-1.0);

	assert(ntask != nTask);
	distributeTaskFrequencies();
}

/* IO */
/* Reads task model, architecture model and power model */
void SchedulabilityAnalysis::readModel(IloEnv &env)
{
	int ntasks, nresources, i;

	if (!fileModel)
		return;

	ifstream file(fileModel);

	if (!file) {
		cerr << "ERROR: could not open file '" << fileModel
			<< "' for reading" << endl;

		throw(-1);
	}

	file >> ntasks >> nresources >> Lp;
	file >> frequencies;
	file >> pdyn ;
	file >> pidle;

	i = 0;
	do {
		class Task t(env);

		file >> t;

		tasks.push_back(t);
	} while (++i < ntasks);

	for (i = 0; i < nresources; i++)
		resourcePriorities.add(-1.0);

	file >> assignment;

	nClusters = assignment.getSize();
	nProcessors = assignment[0].getSize();
	nTasks = assignment[0][0].getSize();
	assert(ntask != nTask);
	nFrequencies = assignment[0][0][0].getSize();
	nResources = nresources;

	loaded = true;
}

/*
 * evaluate_sample_response: evaluates the sample feasibility
 * @parameter tset: set of tasks
 * @parameter runConfig: runConfig information
 * @parameter spread: how good the sample can spread the time for tasks
 * @complexity: O(ntasks)
 */
bool SchedulabilityAnalysis::evaluateResponse(double &spread)
{
	int i, ok;
	char f[10];
	char di, de;
	double s = 0;

	if (runConfig.getList()) {
		for (i = 0; i < nTasks; i++)
			cout << " " << std::fixed << std::setw(21) << std::setprecision(6) << tasks[i].getComputation();

		cout << "   [ ";
	}

	ok = 1;
	for (i = 0; i < nTasks; i++) {
		double dx;
		if (tasks[i].getDeadline() < tasks[i].getResponse() || (tasks[i].getIp() < 0)) {
			di = '<';
			de = '>';
			ok = 0;
			dx = -1.0;
		} else {
			di = ' ';
			de = ' ';
			dx = tasks[i].getDeadline() - tasks[i].getResponse();
		}

		sprintf(f, "%7.2lf", dx);
		if (runConfig.getList())
			cout << di << std::setw(7) << f << de << " ";

		s += dx;
	}

	if (runConfig.getList()) {
		if (ok)
			cout << "]   " << std::setw(-4) << "OK" << " " <<  std::fixed << std::setw(21) <<
					std::setprecision(6) << s;
		else
			cout << "]   " << std::setw(-4) << "NOT" << " " <<  std::fixed << std::setw(21) <<
					std::setprecision(6) << -1.0;
		cout << endl;
	}

	if (ok)
		spread = s;

	return ok;
}

unsigned long long SchedulabilityAnalysis::gcd(unsigned long long a, unsigned long long b)
{
	long long tmp;

	/* As simple as Euclides told me */
	while (b != 0) {

		tmp = b;
		b = a % b;
		a = tmp;
	}

	return a;
}

unsigned long long SchedulabilityAnalysis::gcd_hash(unsigned long long i, unsigned long long j)
{
	unsigned long long a, b;

	a = i; b = j;

	if ((a < MAX_GCD) && (b < MAX_GCD)) {
		return gcd_lookup[a][b];
	}
	if ((a % 2) == 0 && (b % 2) == 0) { /* both even*/
		a = a >> 1; b = b >> 1;
		if ((a < MAX_GCD) && (b < MAX_GCD))
			return gcd_lookup[a][b] * 2;
	} else if ((a % 2) == 1 && (b % 2) == 0) {
		b = b >> 1;
		if ((a < MAX_GCD) && (b < MAX_GCD))
			return gcd_lookup[a][b];
	} else if ((a % 2) == 0 && (b % 2) == 1) {
		a = a >> 1;
		if ((a < MAX_GCD) && (b < MAX_GCD))
			return gcd_lookup[a][b];
	}

	return gcd(i, j);
}

long long SchedulabilityAnalysis::lcm(long long a, long long b)
{
	long long tmp = gcd_hash(a, b);

	if (tmp)
		return (a * b) / tmp;

	return 0;
}

long long SchedulabilityAnalysis::computeLCM(void)
{
	int j, k;
	long long LCM;

	LCM = 0;
	for (j = 0; j < nTasks; j++) {
		long long period = ceil(tasks[j].getPeriod());

		if (LCM == 0)
			LCM = period;

		LCM = lcm(LCM, period);
	}

	return LCM;
}

long long SchedulabilityAnalysis::computeLCM(int c, int p)
{
	int j, k;
	long long LCM;

	LCM = 0;
	for (j = 0; j < nTasks; j++) {
		for (k = 0; k < nFrequencies; k++) {
			if (assignment[c][p][j][k] != 0) {
				long long period = ceil(tasks[j].getPeriod());

				if (LCM == 0)
					LCM = period;

				LCM = lcm(LCM, period);
			}
		}
	}

	return LCM;
}

double SchedulabilityAnalysis::computeSystemEnergy(void)
{
	int s, i, j, k;
	double edyn = 0.0, estat = 0.0;

	double LCM = computeLCM();
	for (s = 0; s < nClusters; s++)
		for (i = 0; i < nProcessors; i++) {
			double ui = 0.0;
			double Pidle = pidle[s][0]; /* dummy getmax() */
			/* TODO: Investigate if LCM must be per processor */

			for (j = 0; j < nTasks; j++) {
				for (k = 0; k < nFrequencies; k++) {
					double u = tasks[j].getUtilization();
					if (assignment[s][i][j][k] != 0) {
						ui += u;
						edyn += floor(LCM / tasks[j].getPeriod()) *
							(u * LCM * pdyn[s][k]);
					}
				}
			}

			if (ui > 0)
				estat += LCM * (1.0 - ui) * Pidle; /* Not smart here */
		}

	return edyn + estat;
}

bool SchedulabilityAnalysis::evaluateUtilization(double bound, double &u)
{
	int s, i, j, k;
	double sum = 0.0;

	if (runConfig.getVerbose()) {
		cout << endl;
		cout << "***************" << endl;
		cout << "* Utilization *" << endl;
		cout << "***************" << endl;
	}
	if (Lp > 0.0)
		computeArchitectureInfluence();
	for (s = 0; s < nClusters; s++)
		for (i = 0; i < nProcessors; i++) {
			double ui = 0.0;
			double si = 0.0;
			int n = 0;

			for (j = 0; j < nTasks; j++)
				for (k = 0; k < nFrequencies; k++)
					if (assignment[s][i][j][k] != 0) {
						ui += tasks[j].getUtilization();
						n++;
						if (Lp > 0.0)
							si += tasks[j].getIa() / tasks[j].getPeriod();
					}
			if (runConfig.getVerbose())
				cout << "U[" << s << "," << i << "] = " <<
					std::fixed << std::setw(21) << std::setprecision(4) <<
					ui << "% + "<< si << "% = " << ui + si << "%" << endl;

			if ((ui + si) > ((double)n * (pow(2.0, 1.0 / (double)n) - 1.0)))
				return false;

			sum += (ui + si);
		}

	u = sum / (nProcessors * nClusters);

	return true;
}

void SchedulabilityAnalysis::computeTotalUtilization(double &u)
{
	int s, i, j, k;
	double sum = 0.0;

	if (Lp > 0.0)
		computeArchitectureInfluence();
	for (s = 0; s < nClusters; s++)
		for (i = 0; i < nProcessors; i++) {
			double ui = 0.0;
			double si = 0.0;
			int n = 0;

			for (j = 0; j < nTasks; j++)
				for (k = 0; k < nFrequencies; k++)
					if (assignment[s][i][j][k] != 0) {
						ui += tasks[j].getUtilization();
						n++;
						if (Lp > 0.0)
							si += tasks[j].getIa() / tasks[j].getPeriod();
					}
			if (runConfig.getVerbose())
				cout << "U[" << s << "," << i << "] = " <<
					std::fixed << std::setw(21) << std::setprecision(4) <<
					ui << "% + "<< si << "% = " << ui + si << "%" << endl;

			sum += (ui + si);
		}

	u = sum / (nProcessors * nClusters);

	if (runConfig.getVerbose()) {
		cout << endl;
		cout << "Total System Utilization: " << u << endl;
	}
}

/*
 * distributeTaskFrequencies: Compute resource priorities
 * @complexity: O(nClusters x nProcessors x ntasks x nFrequencies)
 */
void SchedulabilityAnalysis::distributeTaskFrequencies()
{
	int s, i, j, k;

	for (s = 0; s < nClusters; s++)
		for (i = 0; i < nProcessors; i++)
			for (j = 0; j < nTasks; j++)
				for (k = 0; k < nFrequencies; k++)
					if (assignment[s][i][j][k] != 0)
						tasks[j].setComputation(frequencies[i][k]);
}

/*
 * compute_resource_priorities: Compute resource priorities
 * @parameter tset: a set of tasks
 * @parameter res: array of integer which will be filled with priorities
 * @complexity: O(ntasks x nresources)
 */
void SchedulabilityAnalysis::computeResourcePriorities()
{
	IloNumArray *p;
	int i, j;

	p = &resourcePriorities;
	for (i = 0; i < nResources; i++)
		(*p)[i] = -1;

	/* TODO: Check if we need to do this per processor
	 * For now, assuming the input model makes sure there is a split of
	 * resources per processor. No IPC is allowed.
	 */
	for (i = 0; i < nResources; i++) {
		for (j = 0; j < nTasks; j++) {
			if (tasks[j].getResource(i) > 0.0) {
				if ((*p)[i] < tasks[j].getPriority()) /* t_j has higher prio */
					(*p)[i] = tasks[j].getPriority();
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
void SchedulabilityAnalysis::computeExclusionInfluency()
{
	int i, j, k;

	/* TODO: Check if we need to do this per processor
	 * For now, assuming the input model makes sure there is a split of
	 * resources per processor. No IPC is allowed.
	 */
	for (i = 0; i < nTasks; i++) {
		tasks[i].setIb(0.0);
		for (k = 0; k < nTasks; k++) {
			/* prio(t_i) > prio(t_k) */
			if (tasks[i].getPriority() > tasks[k].getPriority()) {
				for (j = 0; j < nResources; j++) {
					/* If we have high priority */
					if (resourcePriorities[j] >= tasks[i].getPriority() &&
					    tasks[k].getResourceUsage(j) > tasks[i].getIb())
						tasks[i].setIb(tasks[k].getResourceUsage(j));
				}
			}
		}
	}

}

void SchedulabilityAnalysis::computePrecedenceInfluency()
{
	int s, i, j, k, p, t;
	int success;
	double Ip, Ipa;

	for (s = 0; s < nClusters; s++)
		for (i = 0; i < nProcessors; i++)
			for (j = 0; j < nTasks; j++) {
				for (k = 0; k < nFrequencies; k++)
					if (assignment[s][i][j][k] != 0) {
						Ip = tasks[j].getComputation() + tasks[j].getIb() + tasks[j].getIa();
						success = 0;
						while (!success && Ip <= tasks[j].getDeadline()) {
							Ipa = Ip;
							Ip = tasks[j].getComputation() + tasks[j].getIb() + tasks[j].getIa();
							for (p = 0; p < nTasks; p++)
								for (t = 0; t < nFrequencies; t++)
									if (assignment[s][i][p][t] != 0)
										if (tasks[j].getPriority() < tasks[p].getPriority())
											Ip += tasks[p].getPrecedenceInfluence(Ipa);

							success = (fabs(Ip - Ipa) <= DBL_EPSILON);
						}
						tasks[j].setIp(Ip);
					}
			}
}

double SchedulabilityAnalysis::computeTaskArchitectureInfluence(int s, int i, int j)
{
	int p, t, o;
	double numi = 0.0;

	for (o = 0; o < nProcessors; o++) {
		if (o == i)
			continue;
		for (p = 0; p < nTasks; p++)
			for (t = 0; t < nFrequencies; t++)
				if (assignment[s][o][p][t] != 0)
					numi += ceil(tasks[j].getPeriod() / tasks[p].getPeriod()) * 2.0;
	}

	return (2.0 * Lp + numi * Lp);
}

void SchedulabilityAnalysis::computeArchitectureInfluence()
{
	int s, i, j, k;

	for (s = 0; s < nClusters; s++)
		for (i = 0; i < nProcessors; i++)
			for (j = 0; j < nTasks; j++)
				for (k = 0; k < nFrequencies; k++)
					if (assignment[s][i][j][k] != 0)
						tasks[j].setIa(computeTaskArchitectureInfluence(s, i, j));
}

/*
 * compute_sample_analysis: compute influency for each task
 * @parameter tset: set of tasks
 * @parameter res: set of resources
 * @parameter runConfig: determines if output will be verbose
 * @complexity: O(nresources x ntasks ^ 2)
 */
void SchedulabilityAnalysis::computeAnalysis()
{
	if (!loaded)
		return;

	/* O(ntasks x nresources) */
	if (runConfig.getComputeResources())
		computeResourcePriorities();

	if (runConfig.getVerbose())
		/* O(ntasks) + 2xO(nresources) + O(ntasks x nresources) */
		printTaskModel();

	/* O(nresources x ntasks ^ 2) */
	if (runConfig.getComputeResources())
		computeExclusionInfluency();

	if (Lp > 0.0)
		computeArchitectureInfluence();
	/* O(ntasks ^ 2) */
	computePrecedenceInfluency();

	if (runConfig.getVerbose()) {
		/* O(ntasks) */
		printTaskInfluencies();

		/* O(ntasks) */
		printTaskAnalysis();
	}
}

/*
 * print_task_model: prints task model info in a human readable way
 * @parameter tset: set of tasks
 * @parameter res: set of resources
 * @complexity: O(ntasks) + 2xO(nresources) + O(ntasks x nresources)
 */
void SchedulabilityAnalysis::printTaskModel()
{
	int s, i, j, k;

	cout << endl;
	cout << "**************" << endl;
	cout << "* Task Model *" << endl;
	cout << "**************" << endl;
	cout << std::setw(2) << nTasks << " tasks " <<
		std::setw(2) << nResources << " resources " <<
		std::setw(2) << Lp << " Lp " << endl;
	for (s = 0; s < nClusters; s++) {
		cout << "Cluster: " << s << endl;
		for (i = 0; i < nProcessors; i++) {
			cout << "Processor: " << i << endl;
			cout << std::setw(21) << "Task" <<
				std::setw(21) << "Priority" <<
				std::setw(21) << "Computation" <<
				std::setw(21) << "Period" <<
				std::setw(21) << "Deadline" <<
				std::setw(21) << "WCEC" <<
				std::setw(21) << "Frequency" << endl;

			for (j = 0; j < nTasks; j++)
				for (k = 0; k < nFrequencies; k++)
					if (assignment[s][i][j][k] != 0)
						cout << std::setw(21) << j + 1 <<
							tasks[j] <<
							std::setw(21) << frequencies[i][k] << endl;
		}
	}

	if (nResources == 0)
		return;

	cout << endl;
	cout << "**************************" << endl;
	cout << "* Resources in the Model *" << endl;
	cout << "**************************" << endl;
	cout << std::setw(21) << "Task";
	for (i = 0; i < nResources; i++) {
		string tmp("R");
		tmp += i + 1;
		cout << std::setw(21) << tmp;
	}
	cout << endl;

	for (i = 0; i < nTasks; i++) {
		int j;

		cout << std::setw(21) << i + 1;

		for (j = 0; j < nResources; j++)
			cout << std::fixed << std::setw(14) << std::setprecision(6) <<
				tasks[i].getResource(j) * 100 << "%";
		cout << endl;

	}
	cout << "Resources' priorities" << endl;
	for (i = 0; i < nResources; i++)
		cout << "C(R" << std::setw(2) << i + 1 << ") = " <<
			std::setw(21) << resourcePriorities[i];
	cout << endl;

}

/*
 * print_task_influencies: prints each task influence component
 * @parameter tset: set of tasks
 * @complexity: O(ntasks)
 */
void SchedulabilityAnalysis::printTaskInfluencies()
{
	int s, i, j, k;

	cout << endl;
	cout << "*************" << endl;
	cout << "* Influence *" << endl;
	cout << "*************" << endl;
	for (s = 0; s < nClusters; s++) {
		cout << "Cluster: " << s << endl;
		for (i = 0; i < nProcessors; i++) {
			cout << "Processor: " << i << endl;
			cout << std::setw(21) << "Task" <<
				std::setw(21) << "Ai" <<
				std::setw(21) << "Bi" <<
				std::setw(21) << "Ji" <<
				std::setw(21) << "Ii" << endl;

			for (j = 0; j < nTasks; j++)
				for (k = 0; k < nFrequencies; k++)
					if (assignment[s][i][j][k] != 0) {
						cout << std::setw(21) << j + 1 <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) << tasks[j].getIa() <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) << tasks[j].getIb() <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) << tasks[j].getIj() <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) << tasks[j].getIp() <<
							endl;
					}
		}
	}
}

/*
 * print_task_analysis: prints each task computed data
 * @parameter tset: set of tasks
 * @complexity: O(ntasks)
 */
void SchedulabilityAnalysis::printTaskAnalysis()
{
	int s, i, j, k;

	cout << endl;
	cout << "************" << endl;
	cout << "* Analysis *" << endl;
	cout << "************" << endl;
	for (s = 0; s < nClusters; s++) {
		cout << "Cluster: " << s << endl;
		for (i = 0; i < nProcessors; i++) {
			cout << "Processor: " << i << endl;
			cout << std::setw(21) << "Task" <<
				std::setw(21) << "Computation" <<
				std::setw(21) << "Ii" <<
				std::setw(21) << "Ri" <<
				std::setw(21) << "Pi" <<
				std::setw(21) << "Di" <<
				std::setw(21) << "(Di - Ii)" <<
				std::setw(21) << "(Di - Ri)" << endl;

			for (j = 0; j < nTasks; j++)
				for (k = 0; k < nFrequencies; k++)
					if (assignment[s][i][j][k] != 0) {
						cout << std::setw(21) << j + 1 <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) <<
								tasks[j].getComputation() <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) << tasks[j].getIp() <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) <<
								tasks[j].getResponse() <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) <<
								tasks[j].getPeriod() <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) <<
								tasks[j].getDeadline() <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) <<
								tasks[j].getDeadline() - tasks[j].getIp() <<
							std::fixed << std::setw(21) <<
								std::setprecision(6) <<
							tasks[j].getDeadline() - tasks[j].getResponse() <<
							endl;
					}
		}
	}
}
