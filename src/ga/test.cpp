#include <ilcplex/ilocplex.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

typedef IloArray<IloNumArray>    FloatMatrix;
typedef IloArray<IloNumVarArray> NumVarMatrix;

int main(int argc, const char *argv[])
{
	IloEnv env;
	IloNumArray capacity(env), fixedCost(env);
	FloatMatrix cost(env);
	IloIntArray3 x(env);
	const char *filename = ("../../../../examples/data/facility.dat");

	if (argc > 1)
		filename = argv[1];

	ifstream file(filename, ios_base::in);
	if (!file) {
		cerr << "ERROR: could not open file '" << filename
			<< "' for reading" << endl;
		cerr << "usage:   " << argv[0] << " <file>" << endl;
		throw(-1);
	}

	file >> x >>  capacity >> fixedCost >> cost;

	cout << "x" <<  x << endl;
	cout << "Capacity:" << endl << capacity << endl;
	cout << "fixedCost:" << endl << fixedCost << endl;
	cout << "cost:" << endl << cost << endl;

	return 0;
}
