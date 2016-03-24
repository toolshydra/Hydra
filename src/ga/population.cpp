#include <iostream>
#include "fitnesscalcPGA.h"
#include "population.h"
#include "individuals.h"

using namespace std;

Population::Population(int populationSize, bool initialise) : individuals(populationSize)
{
	if (!initialise)
		return;

	generatePopulation(populationSize, populationSize*1000);
}

void Population::generatePopulation(int populationSize, int maxtries)
{
	int i;

	individuals.clear();
	i = 0;
	do {
		Individual ind;

		ind.generateIndividual();
		if (fitnessCalcPGA::isIndividualValid(ind)) {
			individuals.push_back(ind);
			i++;
		}
	} while (--maxtries > 0 && i < populationSize);
}

/* Getters */
Individual Population::getIndividual(int index)
{
	return individuals[index];
}

void Population::setIndividual(int index, Individual indiv)
{
	individuals[index] = indiv;
}

Individual Population::getFittest(void)
{
	Individual fittest = individuals[0];
	int i;

	for (i = 0; i < getSize(); i++) {
		Individual k = getIndividual(i);
		if (fittest.getFitness() <= k.getFitness()) {
			fittest = k;
		}
	}

	return fittest;
}

int Population::getSize(void)
{
	return individuals.size();
}
