#include <vector>
#include <string>
#include "fitnesscalc.h"
#include "individuals.h"

using namespace std;

vector<byte> fitnessCalc::solution(64);

void fitnessCalc::setSolution(vector<byte> newSolution)
{
	solution = newSolution;
}

void fitnessCalc::setSolution(string newSolution)
{
	vector<byte> lsol(newSolution.length());
	int i;

	for (i = 0; i < newSolution.length(); i++) {
		char character = newSolution[i];
		if (character == '0')
			lsol[i] = 0;
		else if (character == '1')
			lsol[i] = 1;
		else
			lsol[i] = 0;
	}
	solution = lsol;
}
int fitnessCalc::getFitness(Individual individual)
{
	int fitness = 0;
	int i;

	for (i = 0; i < individual.getSize() && i < solution.size(); i++)
		if (individual.getGene(i) == solution[i])
			fitness++;

	return fitness;
}

int fitnessCalc::getMaxFitness(void)
{
	return solution.size();
}
