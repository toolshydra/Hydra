#include <iostream>
#include <stdlib.h>
#include "individuals.h"
#include "fitnesscalcPGA.h"

using namespace std;

int Individual::defaultGeneLength = 1000;

Individual::Individual(void) : genes(defaultGeneLength)
{
	int i;

	fitness = 0.0;
	for (i = 0; i < genes.size(); i++)
		genes[i] = 0;
}

Individual::Individual(int size) : genes(size)
{
	int i;

	fitness = 0.0;
	for (i = 0; i < genes.size(); i++)
		genes[i] = 0;
}

void Individual::generateIndividual(void)
{
	fitnessCalcPGA::fitIndividual(this);
}

byte Individual::getGene(int index)
{
	return genes[index];
}

void Individual::setGene(int index, byte gene)
{
	genes[index] = !!gene;
	fitness = 0.0;
}

int Individual::getSize(void)
{
	return genes.size();
}

void Individual::setSize(int size)
{
	return genes.resize(size);
}

double Individual::getFitness(void)
{
	if (fitness >= 0.0 && fitness <= 0.0)
		fitness = fitnessCalcPGA::getFitness(*this);

	return fitness;
}

string Individual::toString(void)
{
	string genestring = "";
	int i;

	for (i = 0; i < getSize(); i++) {
		char b = '0' + getGene(i);
		genestring += b;
	}

	return genestring;
}
