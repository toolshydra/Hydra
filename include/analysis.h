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
#include <string>
#include <vector>
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include <runinfo.h>

#define NTRIES	1000
/* Task data */
struct task {
	double deadline;
	double wcec;
	double computation;
	double Ip;
	double Ib;
	double Ij;
	IloNumArray resources;
};

#define	task_res_use(t, i)		(t.resources[i] * t.computation)
#define precedence_influency(t, w)	(ceil((w + t.Ij) / t.deadline) * \
							t.computation)
#define response(t)			(t.Ip + t.Ij)

class SchedulabilityAnalysis {
/* input data */
private:
	bool loaded;

	vector <struct task> tasks;

	IloNumArray2 frequencies;
	IloNumArray2 voltages;

	IloNumArray resourcePriorities;

	runInfo runConfig;
	string fileModel;


	void computeResourcePriorities();
	void computeExclusionInfluency();
	void computePrecedenceInfluency();
public:
	/* Constructors */
	SchedulabilityAnalysis(runInfo runtime);
	SchedulabilityAnalysis(runInfo runtime, string filename);

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
