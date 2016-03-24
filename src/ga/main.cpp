#include <sys/time.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <iostream>
#include <getopt.h>
#include <errno.h>
#include "fitnesscalcPGA.h"
#include "population.h"
#include "individuals.h"
#include "geneticalgorithm.h"

using namespace std;

static long get_execution_time(struct timeval s, struct timeval e)
{
	struct timeval diff;

	timersub(&e, &s, &diff);

	return diff.tv_sec * 1000000 + diff.tv_usec;
}

int main(int argc, char *argv[])
{
	struct timeval st, e;
	const char *filename = "facility.dat";
	long etimes;
	double f = -1.0;
	int iteration = 100;
	int popSize = 50;
	int equals = 0, max_equals;
	int generationCount = 0;

	if (argc > 1)
		filename = argv[1];

	if (argc > 2)
		iteration = atoi(argv[2]);

	if (argc > 3)  {
		srandom(atoi(argv[3]));
		cout << "Seeded with " << atoi(argv[3]) << endl;
	}

	max_equals = iteration / 10;
	geneticAlgorithm Algorithm;

	fitnessCalcPGA::feedModel(filename);

	gettimeofday(&st, NULL);
	// Create an initial population
	Population myPop(popSize, true);

	if (myPop.getSize() == 0) {
		cout << "Could not generate valid individuals" << endl;
		exit(1);
	}

	if (myPop.getSize() == popSize) {

		// Evolve our population until we reach an optimum solution

		while (generationCount++ < iteration && equals != max_equals) {
			double l = myPop.getFittest().getFitness(); 

			 	//cout << "Generation: " << generationCount << " Fittest: " << l << endl; 
			if (fabs(l - f) <= DBL_EPSILON) {
				equals++;
			} else {
				f = l;
				equals = 0;
			}
			myPop = Algorithm.evolvePopulation(myPop);
		}
	}
	gettimeofday(&e, NULL);
	etimes = get_execution_time(st, e);
	if (fitnessCalcPGA::isIndividualValid(myPop.getFittest())) {
		cout << 1 << endl;
		cout << etimes << endl;
		cout << fitnessCalcPGA::getFOPower(myPop.getFittest()) << endl;
		cout << 0 << endl;
		cout << "Generation: " << generationCount << endl;
		fitnessCalcPGA::dumpConfigurationInfo(myPop.getFittest());
	} else {
		cout << 0 << endl;
		cout << 0 << endl;
		cout << 0 << endl;
		cout << 0 << endl;
		cout << "Solution found is not feasible" << endl;
	}

	return 0;
}
