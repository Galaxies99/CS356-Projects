# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

# define MAX_LINE_BUF 500
# define TRUE 1

// resource number
int resource_num;

// customer number
int customer_num;

// the available amount of each resource
int * available;
// the maximum demand of each customer
int ** maximum;
// the amount currently allocated to each other
int ** allocation;
// the remaining need of each customer
int ** need;
// current capacity
int cur_capacity;


void initialize(int argc, char *argv[]);
void print_state(int op);
void destroy(void);
int parse(char *buf, char *op, int *arg, int * argn);
void upd_need(int ** need, int ** maximum, int ** allocation);
int check_initial_safe(void);
int request_resources(int customer_id, int request[]);
void release_resources(int customer_id, int release[]);


int main(int argc, char *argv[]) {
	initialize(argc, argv);

	print_state(0);

	int res = check_initial_safe();
	if (res) {
		fprintf(stdout, "[Err] Initial state is unsafe! Process will end.\n");
		exit(1);
	}
	
	char buf[MAX_LINE_BUF], op[MAX_LINE_BUF];
	int *arg = (int *) malloc (sizeof(int) * (1 + resource_num)), argn;

	while(TRUE) {
		fprintf(stdout, "Banker >> ");
		fgets(buf, MAX_LINE_BUF, stdin);

		int err = parse(buf, op, arg, &argn);
		if (err) {
			fprintf(stdout, "[Err] Invalid instruction. This instruction will be ignored.\n");
			continue;
		}

		if (strcmp(op, "exit") == 0 && argn == 0) break;
		else if (strcmp(op, "*") == 0 && argn == 0) print_state(2);
		else if (strcmp(op, "RQ") == 0 && argn == resource_num + 1) {
			if (request_resources(arg[0], arg + 1) == -1) fprintf(stdout, "[Log] Request command denied.\n");
			else fprintf(stdout, "[Log] Request command accepted.\n");
		}
		else if (strcmp(op, "RL") == 0 && argn == resource_num + 1) release_resources(arg[0], arg + 1);
		else {
			fprintf(stdout, "[Err] Invalid instruction. This instruction will be ignored.\n");
			continue;
		}
	}

	free(arg);
	
	destroy();
	return 0;
}


// Initialize the arrays according to the input
void initialize(int argc, char *argv[]) {
	// read the initial available resource from the arguments	
	resource_num = argc - 1;
	if (resource_num == 0) {
		fprintf(stderr, "Error: no resource!\n");
		exit(1);
	}

	available = (int *) malloc (sizeof(int) * resource_num);

	for (int i = 1; i < argc; ++ i)
		available[i - 1] = atoi(argv[i]);

	// initial array
	customer_num = 0;
	cur_capacity = 100;
	maximum = (int **) malloc (sizeof(int *) * cur_capacity);

	// read the maximum demand data from file ``maximum.txt''
	FILE *fp = fopen("maximum.txt", "r");
	static int dat;	
	while(~fscanf(fp, "%d", &dat)) {
		// if already full, then double the array.
		if (customer_num == cur_capacity) {
			int ** tem;
			tem = (int **) malloc (sizeof(int *) * cur_capacity * 2);
			for (int i = 0; i < cur_capacity; ++ i) {
				tem[i] = (int *) malloc (sizeof(int) * resource_num);
				for (int j = 0; j < resource_num; ++ j)
					tem[i][j] = maximum[i][j];
				free(maximum[i]);
			}
			free(maximum);
			maximum = tem;
			cur_capacity <<= 1;
		}
		// read the data
		maximum[customer_num] = (int *) malloc (sizeof(int) * resource_num);
		maximum[customer_num][0] = dat;
		for (int i = 1; i < resource_num; ++ i) {
			fscanf(fp, ",%d", &dat);
			maximum[customer_num][i] = dat;
		}
		customer_num ++;
	}
	fclose(fp);

	// allocate the array
	allocation = (int **) malloc (sizeof(int *) * cur_capacity);
	for (int i = 0; i < customer_num; ++ i)
		allocation[i] = (int *) malloc (sizeof(int) * resource_num);
	need = (int **) malloc (sizeof(int *) * cur_capacity);
	for (int i = 0; i < customer_num; ++ i)
		need[i] = (int *) malloc (sizeof(int) * resource_num);
	
	// initialize the array
	for (int i = 0; i < customer_num; ++ i)
		for (int j = 0; j < resource_num; ++ j) 
			allocation[i][j] = 0;
	
	upd_need(need, maximum, allocation);
}

// Print the current state
// op = 0: just output available & maximum; op = 1: also output allocation; op = 2: also output allocation & need
void print_state(int op) {
	fprintf(stdout, "[Log] Current State: \n");
	fprintf(stdout, "  Customer Number = %d\n  Resource Number = %d\n", customer_num, resource_num);
	fprintf(stdout, "  Available = [");
	for (int i = 0; i < resource_num; ++ i)
		fprintf(stdout, "%d%c%c", available[i], (i == resource_num - 1) ? ']' : ',', (i == resource_num - 1) ? '\n' : ' ');

	fprintf(stdout, "  Maximum = [\n");
	for (int i = 0; i < customer_num; ++ i) {
		fprintf(stdout, "    [");
		for (int j = 0; j < resource_num; ++ j)
			fprintf(stdout, "%d%c%c", maximum[i][j], (j == resource_num - 1) ? ']' : ',', (j == resource_num - 1) ? '\n' : ' ');
	}
	fprintf(stdout, "  ]\n");

	if (op >= 1) {
		fprintf(stdout, "  Allocation = [\n");
		for (int i = 0; i < customer_num; ++ i) {
			fprintf(stdout, "    [");
			for (int j = 0; j < resource_num; ++ j)
				fprintf(stdout, "%d%c%c", allocation[i][j], (j == resource_num - 1) ? ']' : ',', (j == resource_num - 1) ? '\n' : ' ');
		}
		fprintf(stdout, "  ]\n");
	}

	if (op >= 2) {
		fprintf(stdout, "  Need = [\n");
		for (int i = 0; i < customer_num; ++ i) {
			fprintf(stdout, "    [");
			for (int j = 0; j < resource_num; ++ j)
				fprintf(stdout, "%d%c%c", need[i][j], (j == resource_num - 1) ? ']' : ',', (j == resource_num - 1) ? '\n' : ' ');
		}
		fprintf(stdout, "  ]\n");
	}
}

// De-allocate the array
void destroy(void) {
	free(available);
	for (int i = 0; i < customer_num; ++ i) {
		free(maximum[i]);
		free(allocation[i]);
		free(need[i]);
	}
	free(maximum);
	free(allocation);
	free(need);
}

// Parse the buffer
int parse(char * buf, char * op, int * arg, int * argn) {
	int hv, tmp, opcnt = 0;	
	hv = 0; tmp = 0;
	(*argn) = -1;	
	for (int i = 0; buf[i]; ++ i) {
		if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n') {
			if (!hv) continue;
			hv = 0;
			if (*argn != -1) {
				if (*argn == resource_num + 1) return 1;				
				arg[*argn] = tmp;
				tmp = 0;
			}
			(*argn) ++;
		} else {
			hv = 1;
			if(*argn == -1) op[opcnt ++] = buf[i];
			else {
				if (buf[i] >= '0' && buf[i] <= '9') tmp = tmp * 10 + buf[i] - '0';
				else return 1;
			}
		}
	}
	op[opcnt] = 0;
	if(hv) {
		if (*argn != -1) {
			if (*argn == resource_num + 1) return 1;				
			arg[*argn] = tmp;
			tmp = 0;
		}
		(*argn) ++;
	}
	return 0;	
}


// Update the need matrix
void upd_need(int ** need, int ** maximum, int ** allocation) {
	for (int i = 0; i < customer_num; ++ i)
		for (int j = 0; j < resource_num; ++ j)
			need[i][j] = maximum[i][j] - allocation[i][j];
}

// Check whether the initial state is safe
int check_initial_safe(void) {
	// |-- if maximum > initial avaible, unsafe.
	for (int i = 0; i < customer_num; ++ i)
		for (int j = 0; j < resource_num; ++ j)
			if(maximum[i][j] > available[j]) return 1;
	// |-- otherwise, safe.
	return 0;
}

// Request the resources
int request_resources(int customer_id, int request[]) {
	int * available_t, * served;

	// |-- Check special cases.
	for (int i = 0; i < resource_num; ++ i)
		if (request[i] > need[customer_id][i]) {
			fprintf(stdout, "[Err] The request should not be greater than need. This request will be ignored.");
			return -1;
		}
	for (int i = 0; i < resource_num; ++ i)
		if (request[i] > available[i]) {
			fprintf(stdout, "[Log] Request CANNOT be granted, because there is not enough available resources.\n");
			return -1;
		}
	
	// |-- Suppose we grant the request, then check the state.
	available_t = (int *) malloc (sizeof(int) * resource_num);
	served = (int *) malloc (sizeof(int) * customer_num);
	for (int i = 0; i < customer_num; ++ i)
		served[i] = 0;
	for (int i = 0; i < resource_num; ++ i) {
		available_t[i] = available[i] - request[i];
		allocation[customer_id][i] += request[i];
	}
	upd_need(need, maximum, allocation);
	
	// |-- Implement the situation.
	int res = 1;
	for (int step = 0; step < customer_num; ++ step) {
		// |---- Find a unserved, feasible customer.		
		int pos = -1;
		for (int i = 0; i < customer_num; ++ i) {
			if (served[i]) continue;
			int flag = 1;
			for (int j = 0; j < resource_num; ++ j)
				if (need[i][j] > available_t[j]) {
					flag = 0;
					break;
				}
			if (flag) {
				pos = i;
				break;
			}
		}
		// |---- Not found, then unsafe.
		if(pos == -1) {
			res = 0;
			break;
		}
		// |---- Serve the customer.
		served[pos] = 1;
		for (int i = 0; i < resource_num; ++ i)
			available_t[i] += allocation[pos][i];
	}
	
	// |-- res = 1, then safe; res = 0, then unsafe.
	if (res) {
		fprintf(stdout, "[Log] Request is granted.\n");
		for (int i = 0; i < resource_num; ++ i)
			available[i] -= request[i];
		free(available_t);
		free(served);
		return 0;
	} else {
		fprintf(stdout, "[Log] Request CANNOT be granted, or the system will become unsafe.\n");
		for (int i = 0; i < resource_num; ++ i)
			allocation[customer_id][i] -= request[i];
		upd_need(need, maximum, allocation);
		free(available_t);
		free(served);
		return -1;
	}
}

// Release the resources
void release_resources(int customer_id, int release[]) {
	// |-- Check special cases.
	for (int i = 0; i < resource_num; ++ i)
		if (release[i] > allocation[customer_id][i]) {
			fprintf(stdout, "[Err] The release should not be greater than allocation. This release will be ignored.\n");
			return ;
		}
	// |-- Release the resources.
	for (int i = 0; i < resource_num; ++ i) {
		available[i] += release[i];
		allocation[customer_id][i] -= release[i];
	}
	upd_need(need, maximum, allocation);
	fprintf(stdout, "[Log] The resources are released.\n");
	return ;
}
