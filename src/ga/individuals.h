#ifndef __INDIVIDUALS_H
#define __INDIVIDUALS_H
#include <string>
#include <vector>

typedef unsigned char byte;

using namespace std;

class Individual {
	private:
		static int defaultGeneLength;

		vector <byte> genes;
		double fitness;
	public:
		Individual(void);
		Individual(int size);
		void generateIndividual(void);
		byte getGene(int index);
		void setGene(int index, byte gene);
		int getSize(void);
		void setSize(int size);
		double getFitness(void);
		string toString(void);
};

#endif
