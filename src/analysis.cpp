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
SchedulabilityAnalysis::SchedulabilityAnalysis(runInfo runtime)
{
	runConfig = runtime;
	fileModel = "model.txt";
	readModel();
}

SchedulabilityAnalysis::SchedulabilityAnalysis(runInfo runtime, string filename)
{
	runConfig = runtime;
	fileModel = filename;
	readModel();
}

/* IO */
/* Reads task model, architecture model and power model */
void SchedulabilityAnalysis::readModel()
{
	if (fileModel.size() == 0)
		return;

	/**/
#if 0

	int i = 0;
	int err;


	if (!tset->tasks) {
		printf("Could not allocate memory for %d tasks.\n",
							tset->ntasks);
		return -ENOMEM;
	}

	/* O(nfrequencies) */
	/* Read array of frequencies */
//	err = read_array(freqs->nfrequencies, &freqs->frequencies);
	if (err < 0) {
		printf("Could not read array of frequencies.\n");
		return err;
	}

	/* Read array of frequencies */
//	err = read_array(freqs->nfrequencies, &freqs->voltages);
	if (err < 0) {
		printf("Could not read array of frequencies.\n");
		return err;
	}

	do {
		struct task *t = tset->tasks + i;

		scanf("%lf %lf %lf", &t->wcec, &t->deadline, &t->Ij);

		/* Read array of resource usages */
//		err = read_array(nresources, &t->resources); /* O(nres) */
		if (err < 0) {
			printf("Could not read array of resources for task"
				" %d.\n", i);
			return err;
		}

	} while (++i < tset->ntasks);

#endif
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

	if (runConfig.list) {
		for (i = 0; i < tasks.size(); i++)
			cout << " " << std::setw(8) << std::setprecision(2) << tasks[i].computation;

		cout << "   [ ";
	}

	ok = 1;
	for (i = 0; i < tasks.size(); i++) {
		double dx;
		if (tasks[i].deadline < response(tasks[i]) || (tasks[i].Ip < 0)) {
			di = '<';
			de = '>';
			ok = 0;
			dx = -1.0;
		} else {
			di = ' ';
			de = ' ';
			dx = tasks[i].deadline - response(tasks[i]);
		}

		sprintf(f, "%7.2lf", dx);
		if (runConfig.list)
			cout << di << std::setw(7) << f << de << " ";

		s += dx;
	}

	if (runConfig.list) {
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
			if (tasks[j].resources[i] > 0.0) {
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
		tasks[i].Ib = 0;
		for (k = i + 1; k < tasks.size(); k++) {
			for (j = 0; j < resourcePriorities.getSize(); j++) {
				/* If we have high priority */
				if (resourcePriorities[j] <= i && task_res_use(tasks[k], j) > tasks[i].Ib)
					tasks[i].Ib = task_res_use(tasks[k], j);
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
		Ip = tasks[i].computation + tasks[i].Ib;
		success = 0;
		while (!success && Ip <= tasks[i].deadline) {
			Ipa = Ip;
			Ip = tasks[i].computation + tasks[i].Ib;
			for (j = i - 1; j >= 0; j--)
				Ip += precedence_influency(tasks[j], Ipa);

			if (almostequal2s_complement(Ip, Ipa, 1 << 22))
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
 * @parameter tset: set of tasks
 * @parameter res: set of resources
 * @parameter runConfig: determines if output will be verbose
 * @complexity: O(nresources x ntasks ^ 2)
 */
void SchedulabilityAnalysis::computeAnalysis()
{

	/* O(ntasks x nresources) */
	computeResourcePriorities();
	if (runConfig.verbose)
		/* O(ntasks) + 2xO(nresources) + O(ntasks x nresources) */
		printTaskModel();

	/* O(nresources x ntasks ^ 2) */
	computeExclusionInfluency();
	/* O(ntasks ^ 2) */
	computePrecedenceInfluency();

	if (runConfig.verbose) {
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
	cout << "Task        Priority        Computation        Deadline" <<
		endl;

	for (i = 0; i < nTasks; i++)
		cout << "T" << std::setw(2) << i + 1 <<
			"       " << std::setw(2) << i <<
			"                " << std::setw(8) << std::setprecision(2) << tasks[i].computation <<
			"                " << std::setw(8) << std::setprecision(2) << tasks[i].deadline <<
			endl;

	cout << endl;
	cout << "**************************" << endl;
	cout << "* Resources in the Model *" << endl;
	cout << "**************************" << endl;
	cout << "Task        ";
	for (i = 0; i < nResources; i++)
		cout << "R" << std::setw(2) << i + 1 << "           ";
	cout << endl;

	for (i = 0; i < nTasks; i++) {
		int j;

		cout << "T" << std::setw(2) << i + 1 << "        ";

		for (j = 0; j < nResources; j++)
			cout << std::setw(8) << std::setprecision(2) <<
				tasks[i].resources[j] * 100 <<"%%        ";
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
	cout << "Task          Bi           Ji           Ii" << endl;
	for (i = 0; i < tasks.size(); i++) {
		cout << "T" << std::setw(2) << i + 1 <<
			"        " << std::setw(8) << std::setprecision(2) << tasks[i].Ib <<
			"        " << std::setw(8) << std::setprecision(2) << tasks[i].Ij <<
			"        " << std::setw(8) << std::setprecision(2) << tasks[i].Ip <<
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
	cout << "Task     Computation               Ii                   "
		"Ri                   Pi              (Pi - Ii)            "
		"(Pi - Ri)" << endl;

	for (i = 0; i < tasks.size(); i++)
		cout << "T" << std::setw(2) << i + 1 <<
			"        " << std::setw(8) << std::setprecision(2) << tasks[i].computation <<
			"                " << std::setw(8) << std::setprecision(2) << tasks[i].Ip <<
			"                " << std::setw(8) << std::setprecision(2) << response(tasks[i]) <<
			"                " << std::setw(8) << std::setprecision(2) << tasks[i].deadline <<
			"                " << std::setw(8) << std::setprecision(2) << tasks[i].deadline - tasks[i].Ip <<
			"                " << std::setw(8) << std::setprecision(2) << tasks[i].deadline - response(tasks[i]) <<
			endl;
}
