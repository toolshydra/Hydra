#ifndef __FITNESSCALC_H
#define __FITNESSCALC_H

#include <vector>
#include "individuals.h"

class fitnessCalc {
	private:
		static std::vector<byte> solution;
	public:
		/* Getters */
		static void setSolution(vector<byte> newSolution);
		static void setSolution(string newSolution);
		static int getFitness(Individual individual);
		static int getMaxFitness(void);
};

#endif
