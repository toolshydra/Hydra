#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>
#include <string>
#include <gcd_hash.h>
#include <task.h>
#include <analysis.h>
#include <runinfo.h>
#include "fitnesscalcPGA.h"
#include "individuals.h"

using namespace std;

IloEnv fitnessCalcPGA::env;
int fitnessCalcPGA::nAgents, fitnessCalcPGA::nTasks, fitnessCalcPGA::nLevels;
IloNumArray2 fitnessCalcPGA::cycles(env), fitnessCalcPGA::voltage(env), fitnessCalcPGA::frequency(env);
IloNumArray fitnessCalcPGA::priority(env);
IloNumArray fitnessCalcPGA::period(env);
IloNumArray fitnessCalcPGA::Deadline(env);
IloNum fitnessCalcPGA::Pidle = 0.260;
double fitnessCalcPGA::LCM;
double fitnessCalcPGA::alpha;

void fitnessCalcPGA::dumpConfigurationInfo(Individual ind)
{
	struct runInfo runtime;
	double sp;
	int s, i, j, k;
	vector <class Task> tasks;
	IloNumArray4 dec(env, 1);

	runtime.setVerbose(true);
	runtime.setList(false);

	for (s = 0; s < 1; s++) {
		dec[s] = IloNumArray3(env, nAgents);
		for (i = 0; i < nAgents; i++) {
			dec[s][i] = IloNumArray2(env, nTasks);
			for (j = 0; j < nTasks; j++) {
				dec[s][i][j] = IloNumArray(env, nLevels, 0, 1, ILOINT);
				for (k = 0; k < nLevels; k++) {
					dec[s][i][j][k] = ind.getGene(i * (nTasks * nLevels) + j * nLevels + k);
				}
			}
		}
	}
	tasks.clear();
	for (j = 0; j < nTasks; j++) {
		Task t(env);

		t.setPriority(priority[j]);
		t.setPeriod(period[j]);
		t.setDeadline(Deadline[j]);
		t.setIp(0.0); /* do not touch for now */
		t.setIb(0.0);/* do not touch for now */
		t.setIa(0.0); /* do not touch for now */
		t.setIj(0.0); /* do not touch for now */
		for (i = 0; i < nAgents; i++)
			for (k = 0; k < nLevels; k++)
				if (dec[0][i][j][k])
					t.setWcec(cycles[i][j]);
		tasks.push_back(t);
	}

	SchedulabilityAnalysis sched(env, runtime, nTasks,
					0, /* nresources */
					0.0, /* Lp */
					frequency, voltage, tasks, dec);

	sched.computeAnalysis();
	sched.evaluateResponse(sp);
	sched.computeTotalUtilization(sp);
	tasks.clear();
	cout << "decision variable: " << endl << dec[0] << endl;
	for (s = 0; s < 1; s++) {
		for (i = 0; i < nAgents; i++) {
			for (j = 0; j < nTasks; j++) {
				dec[s][i][j].end();
			}
			dec[s][i].end();
		}
		dec[s].end();
	}
	dec.end();
}

bool fitnessCalcPGA::isIndividualValid(Individual ind)
{
	struct runInfo runtime;
	double sp;
	int s, i, j, k;
	vector <class Task> tasks;
	IloNumArray4 dec(env, 1);
	bool ret;

	runtime.setVerbose(false);
	runtime.setList(false);

	for (s = 0; s < 1; s++) {
		dec[s] = IloNumArray3(env, nAgents);
		for (i = 0; i < nAgents; i++) {
			dec[s][i] = IloNumArray2(env, nTasks);
			for (j = 0; j < nTasks; j++) {
				dec[s][i][j] = IloNumArray(env, nLevels, 0, 1, ILOINT);
				for (k = 0; k < nLevels; k++) {
					dec[s][i][j][k] = ind.getGene(i * (nTasks * nLevels) + j * nLevels + k);
				}
			}
		}
	}
	tasks.clear();
	for (j = 0; j < nTasks; j++) {
		Task t(env);

		t.setPriority(priority[j]);
		t.setPeriod(period[j]);
		t.setDeadline(Deadline[j]);
		t.setIp(0.0); /* do not touch for now */
		t.setIb(0.0);/* do not touch for now */
		t.setIa(0.0); /* do not touch for now */
		t.setIj(0.0); /* do not touch for now */
		for (i = 0; i < nAgents; i++)
			for (k = 0; k < nLevels; k++)
				if (dec[0][i][j][k])
					t.setWcec(cycles[i][j]);
		tasks.push_back(t);
	}

	SchedulabilityAnalysis sched(env, runtime, nTasks,
					0, /* nresources */
					0.0, /* Lp */
					frequency, voltage, tasks, dec);

	ret = (sched.evaluateUtilization(1.0, sp));
	tasks.clear();
	for (s = 0; s < 1; s++) {
		for (i = 0; i < nAgents; i++) {
			for (j = 0; j < nTasks; j++) {
				dec[s][i][j].end();
			}
			dec[s][i].end();
		}
		dec[s].end();
	}
	dec.end();

	return ret;
}

int fitnessCalcPGA::getTaskGene(int task, Individual ind)
{
	int i, j, k, gene;

	for(i = 0; i < nAgents; i++) {
		for(k = 0; k < nLevels; k++) {
			gene = ind.getGene(i * (nTasks * nLevels) +
						task * nLevels + k);
			if (gene)
				return (i * (nTasks * nLevels) +
					task * nLevels + k);
		}
	}

	cout << "Task " << task << " not assigned"<< endl;

	return -1;
}

void fitnessCalcPGA::feedModel(const char *filename)
{

	ifstream file(filename);

	if (!file) {
		cerr << "ERROR: could not open file '" << filename
			<< "' for reading" << endl;
		throw(-1);
	}

	file >> alpha >> priority >> period >> Deadline >> cycles >> voltage >> frequency;
	nAgents = cycles.getSize();
	nTasks = period.getSize();
	nLevels = frequency[0].getSize();

	LCM = computeLCM(period);
}

void fitnessCalcPGA::fitIndividual(Individual *ind)
{
	int i, j, k;

	ind->setSize(nAgents * nTasks * nLevels);
	for(i = 0; i < nAgents; i++) {
		for(j = 0; j < nTasks; j++) {
			for(k = 0; k < nLevels; k++) {
				ind->setGene(i * (nTasks * nLevels) +
						j * nLevels + k, 0);
			}
		}
	}

	for(j = 0; j < nTasks; j++) {
		int tries = 0;
		double total = 0.0;
		do {
			i = (int)(drand48() * nAgents);
			k = (int)(drand48() * nLevels);

			total = 0.0;
			for(int p = 0; p < nTasks; p++) {
				for(int q = 0; q < nLevels; q++) {
					int index = (i * (nTasks * nLevels) +
						p * nLevels + q);

					total += ind->getGene(index) * (cycles[i][p] / (frequency[i][q])) / period[p];

				}

			}
			int index = (i * (nTasks * nLevels) +
						j * nLevels + k);
			total += ind->getGene(index) * (cycles[i][j] / (frequency[i][k])) / period[j];
		} while (++tries < 1000 && total > 1.0);
		ind->setGene(i * (nTasks * nLevels) +
				j * nLevels + k, 1);
	}

}

double fitnessCalcPGA::getFitness(Individual individual)
{

	/* fi = 1 / Ci */
	return 1.0 / getFOPower(individual);
}

double fitnessCalcPGA::getFOPower(Individual individual)
{
	double power = 0.0;
	int i, j, k;

	for(i = 0; i < nAgents; i++) {
		for(j = 0; j < nTasks; j++) {
			for(k = 0; k < nLevels; k++) {
				power += ((alpha * (LCM / period[j]) * cycles[i][j] *
					(voltage[i][k] * voltage[i][k])) +
					(LCM * (1.0 - (cycles[i][j] / (frequency[i][k]) ) / period[j]) * Pidle)) * individual.getGene(i * (nTasks * nLevels) + j * (nLevels) + k);
			}
		}
	}

	if (power >= 0.0 && power <= 0.0)
		power = std::numeric_limits<double>::max();

	return power;
}
