#include <sys/time.h>
#include <ilcplex/ilocplex.h>
#include <gcd_hash.h>

ILOSTLBEGIN

static long get_execution_time(struct timeval s, struct timeval e)
{
	struct timeval diff;

	timersub(&e, &s, &diff);

	return diff.tv_sec * 1000000 + diff.tv_usec;
}

unsigned long long gcd(unsigned long long a, unsigned long long b)
{
	long long tmp;

	/* As simple as Euclides told me */
	while (b != 0) {

		tmp = b;
		b = a % b;
		a = tmp;
	}

	return a;
}

unsigned long long gcd_hash(unsigned long long i, unsigned long long j)
{
	unsigned long long a, b;

	a = i; b = j;

	if ((a < MAX_GCD) && (b < MAX_GCD)) {
		return gcd_lookup[a][b];
	}
	if ((a % 2) == 0 && (b % 2) == 0) { /* both even*/
		a = a >> 1; b = b >> 1;
		if ((a < MAX_GCD) && (b < MAX_GCD))
			return gcd_lookup[a][b] * 2;
	} else if ((a % 2) == 1 && (b % 2) == 0) {
		b = b >> 1;
		if ((a < MAX_GCD) && (b < MAX_GCD))
			return gcd_lookup[a][b];
	} else if ((a % 2) == 0 && (b % 2) == 1) {
		a = a >> 1;
		if ((a < MAX_GCD) && (b < MAX_GCD))
			return gcd_lookup[a][b];
	}

	return gcd(i, j);
}

long long lcm(long long a, long long b)
{
	long long tmp = gcd_hash(a, b);

	if (tmp)
		return (a * b) / tmp;

	return 0;
}

long long computeLCM(IloNumArray periods)
{
	int j, k;
	long long LCM;

	LCM = 0;
	for (j = 0; j < periods.getSize(); j++) {
		long long period = periods[j];

		if (LCM == 0)
			LCM = period;

		LCM = lcm(LCM, period);
	}

	return LCM;
}

int main(int argc, char **argv)
{
	struct timeval st, e;
	IloEnv env;
	bool good;
	long etimes;
	double energyS;
	try {
		IloInt i, j, k;
		double alpha;
		int nAgents, nTasks, nLevels;
		IloNumArray2 cycles(env), voltage(env), frequency(env);
		IloNumArray priority(env);
		IloNumArray period(env);
		IloNumArray Deadline(env);
		long long LCM;

		const char* filename  = "mgap-rm.dat";
		if (argc > 1)
			filename = argv[1];
		ifstream file(filename);
		if (!file) {
			cerr << "ERROR: could not open file '" << filename
				<< "' for reading" << endl;
			cerr << "usage:   " << argv[0] << " <file>" << endl;
			throw(-1);
		}
		/* float array(nTasks) matrix(nAgents x nTasks) matrix(nAgents x nLevels) matrix(nAgents x nLevels) */
		file >> alpha >> priority >> period >> Deadline >> cycles >> voltage >> frequency;

		nAgents = cycles.getSize();
		nTasks = period.getSize();
		nLevels = frequency[0].getSize();

		LCM = computeLCM(period);

		IloArray<IloArray<IloNumArray> > energy(env, nAgents);
		IloArray<IloArray<IloNumArray> > U(env, nAgents);
		IloArray<IloArray<IloNumArray> > C(env, nAgents);
		for (i = 0; i < nAgents; i++) {
			energy[i] = IloArray<IloNumArray>(env, nTasks);
			U[i] = IloArray<IloNumArray>(env, nTasks);
			C[i] = IloArray<IloNumArray>(env, nTasks);
			for (j = 0; j < nTasks; j++) {
				energy[i][j] = IloNumArray(env, nLevels);
				U[i][j] = IloNumArray(env, nLevels);
				C[i][j] = IloNumArray(env, nLevels);
			}
		}

		for(i = 0; i < nAgents; i++) {
			for(j = 0; j < nTasks; j++) {
				for(k = 0; k < nLevels; k++) {
					energy[i][j][k] = alpha * (LCM / period[j]) * cycles[i][j] *
							(voltage[i][k] * voltage[i][k]);
					C[i][j][k] = cycles[i][j] / (frequency[i][k]);
					U[i][j][k] = C[i][j][k] / period[j];
				}
			}
		}

		IloArray<IloArray<IloNumVarArray> > x(env, nAgents);
		for (i = 0; i < nAgents; i++) {
			x[i] = IloArray<IloNumVarArray>(env, nTasks);
			for (j = 0; j < nTasks; j++)
				x[i][j] = IloNumVarArray(env, nLevels, 0, 1, ILOINT);
		}

		IloModel model(env);
		for(j = 0; j < nTasks; j++) {
			IloExpr v(env);
			for(i = 0; i < nAgents; i++)
				for(k = 0; k < nLevels; k++)
					v += x[i][j][k];
			model.add(v == 1); /* Each task receive only one freq */
			v.end();
		}

		/* Processor Capacity */
		for(j = 0; j < nTasks; j++) {
			for (i = 0; i < nAgents; i++) {
				for (k = 0; k < nLevels; k++) {
					IloExpr times(env, 0.0);
					IloExpr u(env, 0.0);
					/* Response time estimation as per Sj¨odin and Hansson*/

					times = x[i][j][k] * C[i][j][k];

					for (int t = 0; t < nAgents; t++)
						for (int p = 0; p < nTasks; p++) {

							if (priority[p] > priority[j]) { /* Only hp(j)*/
								for (int o = 0; o < nLevels; o++) {
									times += x[t][p][o] * C[t][p][o] * (1.0 - U[t][p][o]);
									u += x[t][p][o] * U[t][p][o];
								}
							}
						}

					model.add(times <= (1.0 - u) * Deadline[j]);
					times.end();
					u.end();
				}
			}
		}

		IloExpr obj(env);
		for(i = 0; i < nAgents; i++)
			for(j = 0; j < nTasks; j++)
				for(k = 0; k < nLevels; k++)
					obj += energy[i][j][k] * x[i][j][k];
		model.add(IloMinimize(env, obj));
		obj.end();

		IloCplex cplex(env);
		cplex.setOut(env.getNullStream());
		cplex.extract(model);
		gettimeofday(&st, NULL);
		cplex.solve();
		gettimeofday(&e, NULL);
		etimes = get_execution_time(st, e);


		if (cplex.getStatus() == IloAlgorithm::Feasible ||
				cplex.getStatus() == IloAlgorithm::Optimal) {
			bool ret = cplex.getObjValue() >= 0;
			if (ret)
				energyS = cplex.getObjValue();
			good = ret;
		} else {
			good = false;
		}

		cplex.end();
		model.end();

		cout << good << endl;
		cout << etimes << endl;
		cout << energyS << endl;

	}
	catch(IloException& e) {
		cerr  << " ERROR: " << e << endl;
	}
	catch(...) {
		cerr  << " ERROR" << endl;
	}
	env.end();
	return 0;
}