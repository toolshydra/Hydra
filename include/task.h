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
	int priority;
	double period;
	double deadline;
	double wcec;
	double computation;
	double Ip;
	double Ib;
	double Ia;
	double Ij;
	IloNumArray resources;
public:
	Task(IloEnv env) :resources(env)
	{
		period = deadline = wcec = computation = Ip = Ib = Ij = 0.0;
	}
	void setDeadline(double deadline)
	{
		this->deadline = deadline;
	}
	void setPeriod(double periodo)
	{
		this->period = period;
	}
	void setWcec(double wcec)
	{
		this->wcec = wcec;
	}
	void setComputation(double frequency)
	{
		this->computation = this->wcec / frequency;
	}
	void setIp(double Ip)
	{
		this->Ip = Ip;
	}
	void setIa(double Ia)
	{
		this->Ia = Ia;
	}
	void setIb(double Ib)
	{
		this->Ib = Ib;
	}
	void setIj(double Ij)
	{
		this->Ij = Ij;
	}
	int getPriority(void)
	{
		return priority;
	}
	double getDeadline(void)
	{
		return deadline;
	}
	double getPeriod(void)
	{
		return period;
	}
	double getWcec(void)
	{
		return wcec;
	}
	double getComputation(void)
	{
		return computation;
	}
	double getUtilization(void)
	{
		return computation / period;
	}
	double getIp(void)
	{
		return Ip;
	}
	double getIb(void)
	{
		return Ib;
	}
	double getIa(void)
	{
		return Ia;
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
		return (ceil((w + Ij) / period) * computation);
	}

	friend ostream& operator <<(ostream &os, const Task &task) {
		os << "       " << std::setw(2)  << task.priority << 

			"                " << std::fixed << std::setw(8) << std::setprecision(2) << task.computation <<
			"                " << std::fixed << std::setw(8) << std::setprecision(2) << task.period <<
			"                " << std::fixed << std::setw(8) << std::setprecision(2) << task.deadline <<
			"                " << std::fixed << std::setw(8) << std::setprecision(2) << task.wcec;
		return os;
	};
	friend istream& operator >>(istream &is, Task &task) {
		is >> task.priority >> task.wcec >> task.period >> task.deadline >> task.Ij >> task.Ib >> task.resources;
		return is;
	};
};

#endif
