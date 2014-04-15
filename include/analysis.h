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

	bool loaded;

	vector <class Task> tasks;

	IloNumArray2 frequencies;
	IloNumArray2 voltages;

	IloNumArray4 assignment;

	IloNumArray resourcePriorities;

	runInfo runConfig;
	const char *fileModel;


	void distributeTaskFrequencies();
	void computeResourcePriorities();
	void computeExclusionInfluency();
	void computePrecedenceInfluency();
public:
	/* Constructors */
	SchedulabilityAnalysis(IloEnv env, runInfo runtime);
	SchedulabilityAnalysis(IloEnv env, runInfo runtime, const char *filename);

	/* Schedulability Analysis */
	void computeAnalysis();
	int evaluateResponse(double *spread);
	void printUtilization();

	/* IO */
	void readModel();
	void printTaskModel();
	void printTaskInfluencies();
	void printTaskAnalysis();
};

#endif
