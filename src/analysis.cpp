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

#include <analysis.h>

#include <string>

/* Constructors */
SchedulabilityAnalysis::SchedulabilityAnalysis(IloEnv env, runInfo runtime)
	:frequencies(env), voltages(env), resourcePriorities(env)
{
	runConfig = runtime;
	if (runConfig.getVerbose())
		cout << runConfig;

	fileModel = "model.txt";
	loaded = false;
	readModel();
}

SchedulabilityAnalysis::SchedulabilityAnalysis(IloEnv env, runInfo runtime, const char *filename)
	:frequencies(env), voltages(env), resourcePriorities(env)
{
	runConfig = runtime;
	if (runConfig.getVerbose())
		cout << runConfig;

	fileModel = filename;
	loaded = false;
	readModel();
}

/* IO */
/* Reads task model, architecture model and power model */
void SchedulabilityAnalysis::readModel()
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

	file >> ntasks >> nresources;
	file >> frequencies;
	file >> voltages;

	i = 0;
	do {
		class Task t(frequencies.getEnv());

		file >> t;

		tasks.push_back(t);
	} while (++i < ntasks);

	for (i = 0; i < nresources; i++)
		resourcePriorities.add(-1.0);

	loaded = true;
}

/*
 * evaluate_sample_response: evaluates the sample feasibility
 * @parameter tset: set of tasks
 * @parameter runConfig: runConfig information
 * @parameter spread: how good the sample can spread the time for tasks
 * @complexity: O(ntasks)
 */
int SchedulabilityAnalysis::evaluateResponse(double *spread)
{
	int i, ok;
	char f[10];
	char di, de;
	double s = 0;

	if (runConfig.getList()) {
		for (i = 0; i < tasks.size(); i++)
			cout << " " << std::setw(8) << std::setprecision(2) << tasks[i].getComputation();

		cout << "   [ ";
	}

	ok = 1;
	for (i = 0; i < tasks.size(); i++) {
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
			cout << "]   " << std::setw(-4) << "OK" << " " <<  std::setw(8) << std::setprecision(2) << s;
		else
			cout << "]   " << std::setw(-4) << "NOT" << " " <<  std::setw(8) << std::setprecision(2) << -1.0;
		cout << endl;
	}

	if (ok)
		*spread = s;

	return ok;
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
	for (i = 0; i < p->getSize(); i++)
		(*p)[i] = -1;

	for (i = 0; i < p->getSize(); i++) {
		for (j = 0; j < tasks.size(); j++) {
			if (tasks[j].getResource(i) > 0.0) {
				(*p)[i] = j;
				break;
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

	for (i = 0; i < tasks.size(); i++) {
		tasks[i].setIb(0.0);
		for (k = i + 1; k < tasks.size(); k++) {
			for (j = 0; j < resourcePriorities.getSize(); j++) {
				/* If we have high priority */
				if (resourcePriorities[j] <= i && tasks[k].getResourceUsage(j) > tasks[i].getIb())
					tasks[i].setIb(tasks[k].getResourceUsage(j));
			}
		}
	}

}

/*
 * compute_precedence_influency: Compute precedence influency of each task
 * @parameter tset: set of tasks
 * @complexity: O(ntasks ^ 2)
 */

/* Usable AlmostEqual function */
static inline int almostequal2s_complement(double A, double B, long long maxulps)
{
    long long aint;
    long long bint;
    long long intdiff = abs(aint - bint);

    /* Make aInt lexicographically ordered as a twos-complement int */
    aint = *(long long *)&A;
    if (aint < 0)
        aint = 0x80000000 - aint;

    /* Make bInt lexicographically ordered as a twos-complement int */
    bint = *(long long*)&B;
    if (bint < 0)
        bint = 0x80000000 - bint;

    intdiff = abs(aint - bint);
    if (intdiff <= maxulps)
        return 1;

    return 0;
}

void SchedulabilityAnalysis::computePrecedenceInfluency()
{
	int i, j;
	int success;
	double Ip, Ipa;

	for (i = 0; i < tasks.size(); i++) {
		Ip = tasks[i].getComputation() + tasks[i].getIb();
		success = 0;
		while (!success && Ip <= tasks[i].getDeadline()) {
			Ipa = Ip;
			Ip = tasks[i].getComputation() + tasks[i].getIb();
			for (j = i - 1; j >= 0; j--)
				Ip += tasks[j].getPrecedenceInfluence(Ipa);

			if (almostequal2s_complement(Ip, Ipa, 1 << 22))
				success = 1;
		}
		if (success)
			tasks[i].setIp(Ip);
		else
			tasks[i].setIp(-1.0);
	}
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
	computeResourcePriorities();
	if (runConfig.getVerbose())
		/* O(ntasks) + 2xO(nresources) + O(ntasks x nresources) */
		printTaskModel();

	/* O(nresources x ntasks ^ 2) */
	computeExclusionInfluency();
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
	int nTasks = tasks.size();
	int nResources = resourcePriorities.getSize();
	int i;

	cout << endl;
	cout << "**************" << endl;
	cout << "* Task Model *" << endl;
	cout << "**************" << endl;
	cout << std::setw(2) << nTasks << " tasks " <<
		std::setw(2) << nResources << " resources" << endl;
	cout << "Task       Priority                Computation            Deadline              WCEC" <<
		endl;

	for (i = 0; i < nTasks; i++)
		cout << "T" << std::setw(2) << i + 1 <<
			"       " << std::setw(2) << i << tasks[i] << endl;

	cout << endl;
	cout << "**************************" << endl;
	cout << "* Resources in the Model *" << endl;
	cout << "**************************" << endl;
	cout << "Task             ";
	for (i = 0; i < nResources; i++)
		cout << "R" << std::setw(2) << i + 1 << "               ";
	cout << endl;

	for (i = 0; i < nTasks; i++) {
		int j;

		cout << "T" << std::setw(2) << i + 1 << "        ";

		for (j = 0; j < nResources; j++)
			cout << std::setw(8) << std::setprecision(2) <<
				tasks[i].getResource(j) * 100 <<"%         ";
		cout << endl;

	}
	cout << "Resources' priorities" << endl;
	for (i = 0; i < nResources; i++)
		cout << "C(R" << std::setw(2) << i + 1 << ") = " <<
			std::setw(2) << resourcePriorities[i] << "        ";
	cout << endl;

}

/*
 * print_task_influencies: prints each task influence component
 * @parameter tset: set of tasks
 * @complexity: O(ntasks)
 */
void SchedulabilityAnalysis::printTaskInfluencies()
{
	int i;

	cout << endl;
	cout << "*************" << endl;
	cout << "* Influence *" << endl;
	cout << "*************" << endl;
	cout << "Task              Bi              Ji              Ii" << endl;
	for (i = 0; i < tasks.size(); i++) {
		cout << "T" << std::setw(2) << i + 1 <<
			"        " << std::setw(8) << std::setprecision(2) << tasks[i].getIb() <<
			"        " << std::setw(8) << std::setprecision(2) << tasks[i].getIj() <<
			"        " << std::setw(8) << std::setprecision(2) << tasks[i].getIp() <<
			endl;
	}
}

/*
 * print_task_analysis: prints each task computed data
 * @parameter tset: set of tasks
 * @complexity: O(ntasks)
 */
void SchedulabilityAnalysis::printTaskAnalysis()
{
	int i;

	cout << endl;
	cout << "************" << endl;
	cout << "* Analysis *" << endl;
	cout << "************" << endl;
	cout << "Task     Computation                     Ii                      "
		"Ri                      Pi                  (Pi - Ii)               "
		"(Pi - Ri)" << endl;

	for (i = 0; i < tasks.size(); i++)
		cout << "T" << std::setw(2) << i + 1 <<
			"        " << std::setw(8) << std::setprecision(2) << tasks[i].getComputation() <<
			"                " << std::setw(8) << std::setprecision(2) << tasks[i].getIp() <<
			"                " << std::setw(8) << std::setprecision(2) <<
							tasks[i].getResponse() <<
			"                " << std::setw(8) << std::setprecision(2) <<
							tasks[i].getDeadline() <<
			"                " << std::setw(8) << std::setprecision(2) <<
							tasks[i].getDeadline() - tasks[i].getIp() <<
			"                " << std::setw(8) << std::setprecision(2) <<
							tasks[i].getDeadline() - tasks[i].getResponse() <<
			endl;
}
