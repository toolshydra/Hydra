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
	bool loaded;

	vector <class Task> tasks;

	IloNumArray2 frequencies;
	IloNumArray2 voltages;

	IloNumArray resourcePriorities;

	runInfo runConfig;
	const char *fileModel;


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
	void evaluateUtilization();

	/* IO */
	void readModel();
	void printTaskModel();
	void printTaskInfluencies();
	void printTaskAnalysis();
};

#endif
