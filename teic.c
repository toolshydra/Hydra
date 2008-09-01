#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#define N_TRIES	5
struct task {
	float deadline;
	float wcec;
	float computation;
	float Ip;
	float Ib;
	float Ij;
	float *resources;
};

/*
 * read_array: Reads an array of floats which may represent a sequence of
 * 		task's properties
 * @parameter n: number of floats
 * @parameter a: array of floats, which will be filled with read values
 * @complexity: O(n)
 */
int read_array (int n, float **a)
{
	int j = 0;
	float *c;

	*a = malloc(sizeof (float) * n);

	if (!a) {
		printf("Could not allocate memory for array\n");
		return -ENOMEM;
	}

	c = *a;

	do {
		scanf("%f", &c[j]);
	} while (++j < n);

	return 0;

}

/*
 * read_task_model: Reads needed info
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks, which will be filled up with task values
 * @parameter nfrequencies: number of frequencies
 * @parameter frequencies: array of floats which will be filled up with
 * 			   available frequencies
 * @parameter nresources: integer which represents the number of resources
 * @complexity: O(nfrequencies) + O(ntasks x nresources)
 */
int read_task_model(int ntasks, struct task **tasks, int nfrequencies,
			float **frequencies, int nresources)
{
	int i = 0;
	int err;

	*tasks = malloc(ntasks * sizeof(**tasks));

	if (!*tasks) {
		printf("Could not allocate memory for %d tasks.\n", ntasks);
		return -ENOMEM;
	}

	err = read_array(nfrequencies, frequencies); /* O(nfrequencies) */
	if (err < 0) {
		printf("Could not read array of frequencies.\n");
		return err;
	}

	do {
		struct task *t = *tasks + i;

		scanf("%f %f %f", &t->wcec, &t->deadline, &t->Ij);

		err = read_array(nresources, &t->resources); /* O(nres) */
		if (err < 0) {
			printf("Could not read array of resources for task"
				" %d.\n", i);
			return err;
		}


	} while (++i < ntasks);

	return 0;
}

/*
 * print_task_model: prints task model info in a human readable way
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter nresources: integer which represents the number of resources
 * @parameter resource_priorities: array of integer with resources priorities
 * @complexity: O(ntasks) + 2xO(nresources) + O(ntasks x nresources)
 */
void print_task_model(int ntasks, struct task *tasks,
			int nresources, int *resource_priorities)
{
	int i;

	printf("\n**************\n");
	printf("* Task Model *\n");
	printf("**************\n");
	printf("%02d tasks, %02d resources\n", ntasks, nresources);
	printf("Task\tPriority\tComputation\tDeadline\n");

	for (i = 0; i < ntasks; i++)
		printf ("T%d\t%d\t\t%.2f\t\t%.2f\n", i + 1, i,
			tasks[i].computation, tasks[i].deadline);

	printf("\n******************\n");
	printf("* Resource Model *\n");
	printf("******************\n");
	printf("Task\t");
	for (i = 0; i < nresources; i++)
		printf("R%d\t", i + 1);
	printf("\n");

	for (i = 0; i < ntasks; i++) {
		int j;

		printf("T%d\t", i + 1);

		for (j = 0; j < nresources; j++)
			printf("%.2f\%\t", tasks[i].resources[j] * 100 );
		printf("\n");

	}
	printf("Resource priorities\n");
	for (i = 0; i < nresources; i++)
		printf("C(R%d) = %d\t", i + 1, resource_priorities[i]);
	printf("\n");

}

/*
 * print_task_influencies: prints each task influence component
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @complexity: O(ntasks)
 */
void print_task_influencies(int ntasks, struct task *tasks)
{
	int i;

	printf("\n*************\n");
	printf("* Influency *\n");
	printf("*************\n");
	printf("Task\tIp\tIb\tIj\tI\n");
	for (i = 0; i < ntasks; i++) {
		float I = tasks[i].Ip + tasks[i].Ib + tasks[i].Ij;
		printf("T%d\t%.2f\t%.2f\t%.2f\t%.2f\n", i + 1, tasks[i].Ip,
			tasks[i].Ib, tasks[i].Ij, I);
	}
}

/*
 * print_task_analysis: prints each task influence component
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter verbose: determines is the output will be verbose
 * @complexity: O(ntasks)
 */
void print_task_analysis(int ntasks, struct task *tasks, int verbose)
{
	int i, ok;
	char f[10];
	char di, de;
	float s = 0;

	if (verbose) {
		printf("\n************\n");
		printf("* Analysis *\n");
		printf("************\n");
		printf("Task\tComputation\tI\t\tResponse\tDeadline\tK (D - I)\tD - R\n");
	}

	printf("\t[ ");
	ok = 1;
	for (i = 0; i < ntasks; i++) {
		float I = tasks[i].Ip + tasks[i].Ib + tasks[i].Ij;
		if (verbose)
			printf("T%d\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\n", i + 1,
				tasks[i].computation, I,
				tasks[i].computation + I,
				tasks[i].deadline,
				tasks[i].deadline - I,
				tasks[i].deadline - (tasks[i].computation + I));
		if (tasks[i].deadline < (tasks[i].computation + I)) {
			di = '<';
			de = '>';
			ok = 0;
		} else {
			di = ' ';
			de = ' ';
		}

		sprintf(f, "%7.2f", tasks[i].deadline - (tasks[i].computation + I));
		printf("%c%7s%c ", di, f, de);

		s += tasks[i].deadline - (tasks[i].computation + I);
	}

	printf("]\t%-4s ", ok ? "OK": "NOT");
	if (ok)
		printf("%7.2f", s);
	printf("\n");
}

/*
 * compute_resource_priorities: Compute resource priorities
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter nresources: integer which represents the number of resources
 * @parameter priorities: array of integer which will be filled with priorities
 * @complexity: O(ntasks x nresources)
 */
void compute_resource_priorities(int ntasks, struct task *tasks,
					int nresources, int **priorities)
{
	int *p;
	int i, j;

	*priorities = malloc(sizeof (int) * nresources);
	p = *priorities;
	memset(p, -1, nresources);

	for (i = 0; i < nresources; i++) {
		for (j = 0; j < ntasks; j++) {
			if (tasks[j].resources[i] > 0.0) {
				p[i] = j;
				break;
			}
		}
	}
}

/*
 * compute_exclusion_influency: Compute exclusion influency of each task
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter nresources: integer which represents the number of resources
 * @parameter priorities: array of integer with resource priorities
 * @complexity: O(nresources x ntasks ^ 2)
 */
void compute_exclusion_influency(int ntasks, struct task *tasks,
					int nresources, int *priorities)
{
	int i, j, k;

	for (i = 0; i < ntasks; i++) {
		tasks[i].Ib = 0;
		for (k = i + 1; k < ntasks; k++) {
			for (j = 0; j < nresources; j++) {
				/* If we have high priority */
				if (priorities[j] <= i) {
					if (tasks[k].resources[j] * tasks[k].computation > tasks[i].Ib)
						tasks[i].Ib = tasks[k].resources[j] * tasks[k].computation;
				}
			}
		}
	}

}

/*
 * compute_precedence_influency: Compute precedence influency of each task
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @complexity: O(ntasks ^ 2)
 */
void compute_precedence_influency(int ntasks, struct task *tasks)
{
	int i, j;
	int success;
	int tries;
	float Ip, Ipa;

	for (i = 0; i < ntasks; i++) {
		tries = 1;
		Ip = tasks[i].computation;
		success = 0;
		while (!success && tries <= N_TRIES) {
			Ipa = Ip;
			Ip = 0;
			for (j = i - 1; j >= 0; j--)
				Ip += ceil((Ipa + tasks[j].Ij) / tasks[j].deadline) * tasks[j].computation;

			if (Ip == 0)
				Ip = Ipa;

			if (Ip == Ipa)
				success = 1;
			tries++;
		}
		if (success)
			tasks[i].Ip = Ip;
		else
			tasks[i].Ip = -1;
	}
}

/*
 * compute_sample_analysis: compute influency for each task
 * @parameter ntasks: number of tasks
 * @parameter tasks: array of tasks
 * @parameter nresources: integer which represents the number of resources
 * @parameter verbose: determines if output will be verbose
 * @complexity: O(nresources x ntasks ^ 2)
 */
void compute_sample_analysis(int ntasks, struct task *tasks,
				int nresources, int verbose)
{
	int *resource_priorities;

 	/* O(ntasks x nresources) */
	compute_resource_priorities(ntasks, tasks, nresources, &resource_priorities);
	if (verbose)
 	/* O(ntasks) + 2xO(nresources) + O(ntasks x nresources) */
		print_task_model(ntasks, tasks, nresources, resource_priorities);

 	/* O(nresources x ntasks ^ 2) */
	compute_exclusion_influency(ntasks, tasks, nresources, resource_priorities);
 	/* O(ntasks ^ 2) */
	compute_precedence_influency(ntasks, tasks);

	if (verbose)
 	/* O(ntasks) */
		print_task_influencies(ntasks, tasks);

 	/* O(ntasks) */
	print_task_analysis(ntasks, tasks, verbose);
}

/*
 * @complexity: O(nfrequencies ^ ntasks) x O(nresources x ntasks ^ 2)
 */
int main(int argc, char *argv[])
{
	int ntasks;
	int nresources;
	int nfrequencies;
	float *frequencies;
	int *resource_priorities;
	struct task *tasks;
	int verbose = 0;

	int i, j;
	int *limits;
	int *ind;

	if (argc > 1)
		verbose = strcmp(argv[1], "-v") == 0;

	if (verbose) {
		printf("*---------------------------------------------------*\n");
		printf("* Scalability Test and Initial Frequency Calculator *\n");
		printf("*---------------------------------------------------*\n");
	}

	scanf("%d %d %d", &ntasks, &nfrequencies, &nresources);

 	/* O(nfrequencies) + O(ntasks x nresources) */
	if (read_task_model(ntasks, &tasks, nfrequencies, &frequencies, nresources) < 0) {
		printf("Error while reading task model\n");
		return -EINVAL;
	}


	ind = malloc(ntasks * sizeof(int));
	if (!ind) {
		printf("Could not allocate memory for indices.\n");
		return -ENOMEM;
	}
	memset(ind, ntasks, 0);

	limits = malloc(ntasks * sizeof(int));
	if (!limits) {
		printf("Could not allocate memory for indices.\n");
		return -ENOMEM;
	}
	for (i = 0 ; i < ntasks; i++)
		limits[i] = nfrequencies;

	for (i = nfrequencies - 1; i >= 0 ; i--) {
		for (j = 0; j < ntasks; j++) {
			tasks[j].computation = tasks[j].wcec / frequencies[i];
			if (tasks[j].computation > tasks[j].deadline)
				limits[j] = i;
		}
	}

	j = 1;
	while (ind[0] < limits[0]) {

		printf("%03d -", j++);
		for (i = 0; i < ntasks; i++) {
			tasks[i].computation = tasks[i].wcec / frequencies[ind[i]];
			printf(" %6.2f", tasks[i].computation);
		}
		printf("\t");

		compute_sample_analysis(ntasks, tasks, nresources, verbose);

		i = ntasks - 1;
		while (++ind[i] >= limits[i]) {
			if (ind[0] == limits[0])
				break;
			ind[i] = 0;
			i--;
		}
	}

	return 0;
}
