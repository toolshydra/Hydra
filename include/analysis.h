/*
 * include/analysis.h
 *
 * Copyright (C) 2008 Eduardo Valentin <edubezval@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <list>
#include <vector>
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include <runinfo.h>
#include <task.h>

#define NTRIES	1000
class SchedulabilityAnalysis {
/* input data */
private:
	int nClusters;
	int nProcessors;
	int nTasks;
	int nFrequencies;
	int nResources;
	double Lp; /* DVFS Worst Switching delay */

	bool loaded;

	vector <class Task> tasks;

	IloNumArray2 frequencies;
	IloNumArray2 voltages;
	IloNumArray2 pdyn;
	IloNumArray2 pidle;

	IloNumArray4 assignment;

	IloNumArray resourcePriorities;

	runInfo runConfig;
	const char *fileModel;


	void distributeTaskFrequencies();
	void computeResourcePriorities();
	void computeExclusionInfluency();
	void computePrecedenceInfluency();
	void computeArchitectureInfluence();
	double computeTaskArchitectureInfluence(int s, int i, int j);
	unsigned long long gcd(unsigned long long a, unsigned long long b);
	unsigned long long gcd_hash(unsigned long long i, unsigned long long j);
	long long lcm(long long a, long long b);
	/* Compute LCM of all periods of tasks in processor p of cluster c */
	long long computeLCM(int c, int p);
	long long computeLCM(void);
public:
	/* Constructors */
	SchedulabilityAnalysis(IloEnv env, runInfo runtime);
	SchedulabilityAnalysis(IloEnv env, runInfo runtime, const char *filename);
	SchedulabilityAnalysis(IloEnv env, runInfo runtime, int ntask,
		int nresources, double lp, IloNumArray2 freqs, IloNumArray2 volts,
		vector <class Task> tset, IloNumArray4 assig);

	SchedulabilityAnalysis(IloEnv env, runInfo runtime, int ntask,
		int nresources, double lp, IloNumArray2 freqs, IloNumArray2 power_dyn,
		IloNumArray2 power_idle, vector <class Task> tset, IloNumArray4 assig);
	/* Schedulability Analysis */
	void computeAnalysis();
	bool evaluateResponse(double &spread);
	bool evaluateUtilization(double bound, double &u);
	double computeSystemEnergy(void);

	/* IO */
	void readModel();
	void printTaskModel();
	void printTaskInfluencies();
	void printTaskAnalysis();
};

#endif
