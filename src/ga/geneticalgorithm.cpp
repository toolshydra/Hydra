#include <cmath>
#include <stdlib.h>
#include "population.h"
#include "fitnesscalcPGA.h"
#include "individuals.h"
#include "geneticalgorithm.h"
#include <iostream>

geneticAlgorithm::geneticAlgorithm(void)
{
	uniformRate = 0.5;
	mutationRate = 0.0;
	tournamentSize = 5;
	elitism = true;
}

Population geneticAlgorithm::evolvePopulation(Population pop)
{
	Population newPopulation(pop.getSize(), false);
	int i, elitismOffset;

	if (elitism)
		newPopulation.setIndividual(0, pop.getFittest());

	// crossover population
	if (elitism)
		elitismOffset = 1;
	else
		elitismOffset = 0;

	// Creating individuals with crossover
	for (i = elitismOffset; i < pop.getSize(); i++) {
		Individual indiv1 = tournamentSelection(pop);
		Individual indiv2 = tournamentSelection(pop);
		Individual newIndiv = crossover(indiv1, indiv2);

		newPopulation.setIndividual(i, newIndiv);
	}

	// mutate population
	for (i = elitismOffset; i < newPopulation.getSize(); i++) {
		Individual ind = newPopulation.getIndividual(i);

		mutate(&ind);

		newPopulation.setIndividual(i, ind);
	}

	return newPopulation;
}

Individual geneticAlgorithm::crossover(Individual indiv1, Individual indiv2)
{
	Individual newSol(indiv1.getSize());
	int nTasks = fitnessCalcPGA::getNTasks();
	bool valid = false;
	int found;
	int j;

	found = 0;

	// Pivoting
	while (found < nTasks && !valid) {
		// get lower tasks from Indiv1
		for (j = 0; j < found; j++) {
			int gene;

			gene = fitnessCalcPGA::getTaskGene(j, indiv1);	
			newSol.setGene(gene, 1);

		}
		// get upper tasks from Indiv2
		for (j = found; j < nTasks; j++) {
			int gene;

			gene = fitnessCalcPGA::getTaskGene(j, indiv2);	
			newSol.setGene(gene, 1);

		}
		valid = fitnessCalcPGA::isIndividualValid(newSol);
		found++; 
	}

	if (!valid) {
		double fo1, fo2;

		fo1 = fitnessCalcPGA::getFOPower(indiv1);
		fo2 = fitnessCalcPGA::getFOPower(indiv2);
		if (fo1 > fo2)
			newSol = fo2;
		else
			newSol = fo1;
	}

	return newSol;
}

void geneticAlgorithm::mutate(Individual *indiv)
{
	int nAgents= fitnessCalcPGA::getNAgents();
	int nTasks = fitnessCalcPGA::getNTasks();
	int nLevels = fitnessCalcPGA::getNLevels();
	int j;

	for (j = 0; j < nTasks; j++) {
		if (((double)random() / (RAND_MAX)) <= mutationRate) {
			int a  = (int)drand48() * nAgents;
			int l  = (int)drand48() * nLevels;
			indiv->setGene(fitnessCalcPGA::getTaskGene(j, *indiv), 0);
			indiv->setGene(a * (nTasks * nLevels) + j * nLevels + l, 1);
		}
	}

}

Individual geneticAlgorithm::tournamentSelection(Population pop)
{
	Population tournament(tournamentSize, false);
	int i;

	for (i = 0; i < tournamentSize; i++) {
		int randomomId = (int) (((double)random() / (RAND_MAX)) * (double) pop.getSize());

		tournament.setIndividual(i, pop.getIndividual(randomomId));
	}

	return tournament.getFittest();
}
