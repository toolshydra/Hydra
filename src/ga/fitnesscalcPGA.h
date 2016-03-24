#ifndef __FITNESSCALCPGA_H
#define __FITNESSCALCPGA_H

#include <vector>
#include <ilcplex/ilocplex.h>
#include "individuals.h"

class fitnessCalcPGA {
	private:
		static IloEnv env;
		static int nAgents, nTasks, nLevels;
		static IloNumArray2 cycles, voltage, frequency;
		static IloNumArray priority;
		static IloNumArray period;
		static IloNumArray Deadline;
		static IloNum Pidle;
		static double LCM;
		static double alpha;
	public:
		/* Getters */
		static int getNTasks()
		{
			return nTasks;
		}
		static int getNAgents()
		{
			return nAgents;
		}
		static int getNLevels()
		{
			return nLevels;
		}
		static void dumpConfigurationInfo(Individual ind);
		static bool isIndividualValid(Individual ind);
		static int getTaskGene(int task, Individual ind);
		static int selectFreeGene(Individual ind);
		static void feedModel(const char *filename);
		static void fitIndividual(Individual *individual);
		static double getFitness(Individual individual);
		static double getFOPower(Individual individual);
};

#endif
