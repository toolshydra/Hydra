#include <sys/time.h>
#include <ilcplex/ilocplex.h>
#include <getopt.h>
#include <errno.h>
#include <gcd_hash.h>

ILOSTLBEGIN

static const char *short_options = "hstm:";
static const struct option long_options[] = {
	{ "help",     0, NULL, 'h' },
	{ "model",     0, NULL, 'm' },
	{ "solution",     0, NULL, 's' },
	{ "statistics",     0, NULL, 't' },
	{ NULL,       0, NULL, 0   },   /* Required at end of array.  */
};

static void print_usage(char *program_name)
{
	printf("Usage: %s  options\n", program_name);
	printf(
	"  -h  --help                             Display this usage information.\n"
	"  -m  --model=<modelfile>                Read model specification from modelfile.\n"
	"  -s  --solution                         Print at the end the found solution.\n"
	"  -t  --statistics                       Print at the end the feasibility, processing time, and minimum energy found.\n");

}

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
	const char* filename  = "mgap-rm.dat";
	IloEnv env;
	bool good, stats = false, solution = false;
	long etimes;
	double energyS;
	int next_option;

	/* Read command line options */
	do {
		next_option = getopt_long (argc, argv, short_options,
						long_options, NULL);
		switch (next_option) {
		default:    /* Something else: unexpected.  */
		case '?':   /* The user specified an invalid option.  */
		case 'h':   /* -h or --help */
			print_usage(argv[0]);
			return 0;
		case 'm':   /* -m or --model */
			if (!optarg) {
				fprintf(stderr, "Specify file with model.\n");
				print_usage(argv[0]);
				return -EINVAL;
			}
			filename = optarg;
			break;
		case 's':   /* -s or --solution */
			solution = true;
			break;
		case 't':   /* -t or --statistics */
			stats = true;
			break;
		case -1:    /* Done with options.  */
			break;
		}
	} while (next_option != -1);

	try {
		IloInt i, j, k;
		double alpha, bound;
		int nAgents, nTasks, nLevels;
		IloNumArray2 cycles(env), voltage(env), frequency(env);
		IloNumArray period(env);
		long long LCM;
		ifstream file(filename);

		if (!file) {
			cerr << "ERROR: could not open file '" << filename
				<< "' for reading" << endl;
			cerr << "usage:   " << argv[0] << " <file>" << endl;
			throw(-1);
		}
		/* float array(nTasks) matrix(nAgents x nTasks) matrix(nAgents x nLevels) matrix(nAgents x nLevels) */
		file >> alpha >> bound >> period >> cycles >> voltage >> frequency;

		nAgents = cycles.getSize();
		nTasks = period.getSize();
		nLevels = frequency[0].getSize();

		LCM = computeLCM(period);

		IloArray<IloArray<IloNumArray> > cost(env, nAgents);
		IloArray<IloArray<IloNumArray> > req(env, nAgents);
		for (i = 0; i < nAgents; i++) {
			cost[i] = IloArray<IloNumArray>(env, nTasks);
			req[i] = IloArray<IloNumArray>(env, nTasks);
			for (j = 0; j < nTasks; j++) {
				cost[i][j] = IloNumArray(env, nLevels);
				req[i][j] = IloNumArray(env, nLevels);
			}
		}

		for(i = 0; i < nAgents; i++) {
			for(j = 0; j < nTasks; j++) {
				for(k = 0; k < nLevels; k++) {
					cost[i][j][k] = alpha * (LCM / period[j]) * cycles[i][j] *
							(voltage[i][k] * voltage[i][k]);
					req[i][j][k] = cycles[i][j] / (frequency[i][k] * period[j]);
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
		for(i = 0; i < nAgents; i++) {
			IloExpr v(env);
			for(j = 0; j < nTasks; j++) {
				for(k = 0; k < nLevels; k++)
					v += req[i][j][k] * x[i][j][k];
			}
			model.add(v <= bound); /* Each agent has a budget */
			v.end();
		}

		IloExpr obj(env);
		for(i = 0; i < nAgents; i++)
			for(j = 0; j < nTasks; j++)
				for(k = 0; k < nLevels; k++)
					obj += cost[i][j][k] * x[i][j][k];
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

		if (stats) {
			cout << good << endl;
			cout << etimes << endl;
			cout << energyS << endl;
		}

		if (solution) {
			cout << "Optimal System Energy: " << cplex.getObjValue() << endl;
			for(i = 0; i < nAgents; i++)
				for(j = 0; j < nTasks; j++)
					for(k = 0; k < nLevels; k++)
						if (cplex.getValue(x[i][j][k]) == 1)
							cout << "Task[" << j
								<< "] runs in processor " << i
								<< " at level [" << k << "] ("
								<< frequency[i][k] << "Hz@"
								<< voltage[i][k] << "V)" << endl;
		}

		cplex.end();
		model.end();
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
