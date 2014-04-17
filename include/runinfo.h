/*
 * include/runinfo.h
 *
 * Copyright (C) 2014 Eduardo Valentin <edubezval@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef RUNINFO_H
#define RUNINFO_H

#include <iostream>

using namespace std;

/* Runtime data */
class runInfo {
private:
	bool summary;			/* print a summary in the end */
	bool verbose;			/* verbose execution */
	bool list;			/* list samples */
	bool computeResources;		/* compute Bi based on input resources */

public:
	runInfo(void)
	{
		summary = verbose = list = computeResources = false;
	}
	void setSummary(bool summary)
	{
		this->summary = summary;
	};			/* print a summary in the end */
	void setVerbose(bool verbose)
	{
		this->verbose = verbose;
	};			/* verbose execution */
	void setList(bool list)
	{
		this->list = list;
	};			/* list samples */
	void setComputeResources(bool computeResources)
	{
		this->computeResources = computeResources;
	};			/* Compute Bi based on input resources */
	bool getSummary(void)
	{
		return summary;
	};			/* print a summary in the end */
	bool getVerbose(void)
	{
		return verbose;
	};			/* verbose execution */
	bool getList(void)
	{
		return list;
	};			/* list samples */
	bool getComputeResources(void)
	{
		return computeResources;
	};			/* Compute Bi based on input resources */

	friend ostream& operator <<(ostream &os, const runInfo &ri) {
		os << "Summary = " << ri.summary << endl <<
			"Verbose = " << ri.verbose << endl <<
			"List = " << ri.list << endl <<
			"Compute Resource Bi = " << ri.computeResources << endl;
	};
};
#endif
