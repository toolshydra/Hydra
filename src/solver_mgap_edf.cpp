#include <ilcplex/ilocplex.h>

ILOSTLBEGIN


int main(int argc, char **argv)
{
	IloEnv env;
	try {
		IloInt i, j, k;
		double alpha;
		int nAgents, nTasks, nLevels;
		IloNumArray2 cycles(env), voltage(env), frequency(env);
		IloNumArray period(env);

		const char* filename  = "mgap-edf.dat";
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
		file >> alpha >> period >> cycles >> voltage >> frequency;

		nAgents = cycles.getSize();
		nTasks = period.getSize();
		nLevels = frequency[0].getSize();

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
					cost[i][j][k] = alpha * cycles[i][j] *
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
			model.add(v <= 1); /* Each agent has a budget */
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
		cplex.extract(model);
		cplex.solve();

		cplex.out() << "Solution status: " << cplex.getStatus() << endl;

		IloNum tolerance = cplex.getParam(IloCplex::EpInt);
		cplex.out() << "Optimal System Energy: " << cplex.getObjValue() << endl;
		for(i = 0; i < nAgents; i++)
			for(j = 0; j < nTasks; j++)
				for(k = 0; k < nLevels; k++)
					if (cplex.getValue(x[i][j][k]) == 1)
						cplex.out() << "Task[" << j
							<< "] runs in processor " << i
							<< " at level [" << k << "] ("
							<< frequency[i][k] << "Hz@"
							<< voltage[i][k] << "V)" << endl;
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
