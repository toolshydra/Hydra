#ifndef __GENETIC_ALGORITHM_H
#define __GENETIC_ALGORITHM_H

#include "individuals.h"

class geneticAlgorithm {
	private:
		double uniformRate;
		double mutationRate;
		int tournamentSize;
		bool elitism;
	public:
		geneticAlgorithm(void);
		Population evolvePopulation(Population pop);
		Individual crossover(Individual indiv1, Individual indiv2);
		void mutate(Individual *indiv);
		Individual tournamentSelection(Population pop);
};

#endif
