#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#define N_TRIES	5
struct task {
	float deadline;
	float computation;
	float Ip;
	float Ib;
	float Ij;
	float *resources;
};

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

int read_task_model(int ntasks, int nresources,
			struct task **tasks)
{
	int i = 0;
	int err;

	*tasks = malloc(ntasks * sizeof(**tasks));

	if (!*tasks) {
		printf("Could not allocate memory for %d tasks.\n", ntasks);
		return -ENOMEM;
	}

	do {
		struct task *t = *tasks + i;

		scanf("%f %f %f", &t->computation, &t->deadline, &t->Ij);

		err = read_array(nresources, &t->resources);
		if (err < 0) {
			printf("Could not read array of resources for task %d.\n", i);
			return err;
		}


	} while (++i < ntasks);

	return 0;
}

void print_task_model(int ntasks, int nresources,
			struct task *tasks, int *resource_priorities)
{
	int i;

	printf("\n**************\n");
	printf("* Task Model *\n");
	printf("**************\n");
	printf("%02d tasks, %02d resources\n", ntasks, nresources);
	printf("Task\tPriority\tComputation\tDeadline\tJitter\n");

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

void print_task_analysis(int ntasks, struct task *tasks)
{
	int i;

	printf("\n************\n");
	printf("* Analysis *\n");
	printf("************\n");
	printf("Task\tComputation\tI\t\tResponse\tDeadline\tK (D - I)\tD - R\n");
	for (i = 0; i < ntasks; i++) {
		float I = tasks[i].Ip + tasks[i].Ib + tasks[i].Ij;
		printf("T%d\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\n", i + 1,
			tasks[i].computation, I,
			tasks[i].computation + I,
			tasks[i].deadline,
			tasks[i].deadline - I,
			tasks[i].deadline - (tasks[i].computation + I));
	}
}

/*
 * Compute resource priorities
 */
void compute_resource_priorities(int ntasks, int nresources,
					struct task *tasks, int **priorities)
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

void compute_exclusion_influency(int ntasks, int nresources,
					struct task *tasks, int *priorities)
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

int main(int argc, char *argv[])
{
	int ntasks;
	int nresources;
	int *resource_priorities;
	struct task *tasks;
	int verbose = 0;

	if (argc > 1)
		verbose = strcmp(argv[1], "-v") == 0;

	if (verbose) {
		printf("*---------------------------------------------------*\n");
		printf("* Scalability Test and Initial Frequency Calculator *\n");
		printf("*---------------------------------------------------*\n");
	}

	scanf("%d %d", &ntasks, &nresources);

	if (read_task_model(ntasks, nresources, &tasks) < 0) {
		printf("Error while reading task model\n");
		return -EINVAL;
	}
	compute_resource_priorities(ntasks, nresources, tasks, &resource_priorities);
	if (verbose)
		print_task_model(ntasks, nresources, tasks, resource_priorities);

	compute_exclusion_influency(ntasks, nresources, tasks, resource_priorities);
	compute_precedence_influency(ntasks, tasks);

	if (verbose)
		print_task_influencies(ntasks, tasks);

	print_task_analysis(ntasks, tasks);

	return 0;
}
