#ifndef __POPULATION_H
#define __POPULATION_H

#include <vector>
#include "individuals.h"

class Population {
	private:
		std::vector<Individual> individuals;
	public:
		Population(int polulationSize, bool initialise);
		void generatePopulation(int polulationSize, int maxtries);
		/* Getters */
		Individual getIndividual(int index);
		void setIndividual(int index, Individual indiv);
		Individual getFittest(void);
		int getSize(void);
};

#endif
