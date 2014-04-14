/*
 * include/task.h
 *
 * Copyright (C) 2014 Eduardo Valentin <edubezval@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef TASK_H
#define TASK_H

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include <cmath>
#include <iostream>

using namespace std;

/* Task data */
class Task {
private:
	double deadline;
	double wcec;
	double computation;
	double Ip;
	double Ib;
	double Ij;
	IloNumArray resources;
public:
	Task(IloEnv env) :resources(env)
	{
		deadline = wcec = computation = Ip = Ib = Ij = 0.0;
	}
	void setDeadline(double deadline)
	{
		this->deadline = deadline;
	}
	void setWcec(double wcec)
	{
		this->wcec = wcec;
	}
	void setComputation(double computation)
	{
		this->computation = computation;
	}
	void setIp(double Ip)
	{
		this->Ip = Ip;
	}
	void setIb(double Ib)
	{
		this->Ib = Ib;
	}
	void setIj(double Ij)
	{
		this->Ij = Ij;
	}
	double getDeadline(void)
	{
		return deadline;
	}
	double getWcec(void)
	{
		return wcec;
	}
	double getComputation(void)
	{
		return computation;
	}
	double getIp(void)
	{
		return Ip;
	}
	double getIb(void)
	{
		return Ib;
	}
	double getIj(void)
	{
		return Ij;
	}
	double getResource(int i)
	{
		return resources[i];
	}
	double getResponse(void)
	{
		return (Ip + Ij);
	}
	double getResourceUsage(int i)
	{
		return (resources[i] * computation);
	}
	double getPrecedenceInfluence(double w)
	{
		return (ceil((w + Ij) / deadline) * computation);
	}

	friend ostream& operator <<(ostream &os, const Task &task) {
		os << "                " << std::setw(8) << std::setprecision(2) << task.computation <<
			"                " << std::setw(8) << std::setprecision(2) << task.deadline <<
			"                " << std::setw(8) << std::setprecision(2) << task.wcec;
		return os;
	};
	friend istream& operator >>(istream &is, Task &task) {
		is >> task.wcec >> task.deadline >> task.Ij >> task.resources;
		return is;
	};
};

#endif
