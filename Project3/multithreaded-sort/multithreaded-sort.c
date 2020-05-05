// Note: remember to add '-g -lpthread' when compiling.
# include <stdio.h>
# include <pthread.h>
# include <stdlib.h>

// the size of the array.
int n;
// original array.
int *arr;
// result array.
int *res;

struct parameters {
	// the sorting thread will sort arr[begin ... end]
	int begin, end;
};

struct merge_parameters {
	// the merging thread will merge arr[begin ... mid] and arr[mid + 1, end]
	int begin, mid, end;
};

int qsort_cmp(const void *a, const void *b) {
	return * (int *) a - * (int *) b;
}

void* sorting_routine(void *arg) {
	struct parameters *data = (struct parameters *) arg;
	
	if (data -> end - data -> begin < 0) return NULL;

	// Quick sort provided by <stdlib.h>.
	qsort(arr + data -> begin, data -> end - data -> begin + 1, sizeof(int), qsort_cmp);
	
	// Return.
	return NULL;
}

void* merging_routine(void *arg) {
	struct merge_parameters *data = (struct merge_parameters *) arg;
	// Merge two sorted arrays.
	// |-- pos of result
	int res_pos = data -> begin;
	// |-- pos of two arrays.
	int pos0 = data -> begin, pos1 = data -> mid + 1;

	// |-- comparing & merging.
	while (pos0 <= data -> mid && pos1 <= data -> end) {
		if (arr[pos0] <= arr[pos1]) res[res_pos ++] = arr[pos0 ++];
		else res[res_pos ++] = arr[pos1 ++];
	}
	// |-- the rest of the elements.
	while (pos0 <= data -> mid)
		res[res_pos ++] = arr[pos0 ++];
	while (pos1 <= data -> end)
		res[res_pos ++] = arr[pos1 ++];
	
	// Check the result.
	if (res_pos != data -> end + 1) {
		printf("Error: unexpected error occurs when merging.\n");
		exit(1);	
	}
	
	// Return.
	return NULL;
}
	
int main(void) {
	int err;
	
	// Input
	printf("Please input the length of the array (0 <= n <= 1000000): ");
	scanf("%d", &n);
  
	if(n < 0 || n > 1000000) {
		printf("Error: n should be in range [0, 1000000]!\n");
		exit(1);
	}

	// Allocate spaces for arrays.  
	arr = (int *) malloc (n * sizeof(int)); 
	res = (int *) malloc (n * sizeof(int));

	// Input the elements or Generate the elements.
	char opt[5];
	printf("Do you want to generate the random elements automatically (y/n): ");
	scanf("%s", opt);
	if(opt[0] == 'y') {
		for (int i = 0; i < n; ++ i)
			arr[i] = rand() % 1000;
		printf("The original array: \n");
		for (int i = 0; i < n; ++ i)
			printf("%d ", arr[i]);
		printf("\n");
	} else if (opt[0] == 'n') {  
		printf("Please input the array elements: \n");
		for (int i = 0; i < n; ++ i)
			scanf("%d", &arr[i]);
	} else {
		printf("Error: invalid input!\n");
		exit(1);
	}

	// Prepare parameters for sorting threads.
	struct parameters param[2];
	// |-- thread 0 parameters  
	param[0].begin = 0;
	param[0].end = n / 2;
	// |-- thread 1 parameters
	param[1].begin = n / 2 + 1;
	param[1].end = n - 1;
	
	// Create sorting threads & passing parameters.
	pthread_t sorting_thread[2];
	for (int i = 0; i < 2; ++ i) {
  	err = pthread_create(&sorting_thread[i], NULL, sorting_routine, &param[i]);
		if (err) {
			printf("Error: create thread failed!\n");
			exit(1);
		}
	}

	// Sorting threads end.
	void *output;
	for (int i = 0; i < 2; ++ i) {
		err = pthread_join(sorting_thread[i], &output);
		if (err) {
			printf("Error: thread join failed!\n");
			exit(1);
		}
	}

	// Prepare parameters for merging thread.
	struct merge_parameters m_param;
	m_param.begin = 0;
	m_param.mid = n / 2;
	m_param.end = n - 1;

	// Create merging thread & passing parameters.
	pthread_t merging_thread;
	err = pthread_create(&merging_thread, NULL, merging_routine, &m_param);
	if (err) {
		printf("Error: create thread failed!\n");
		exit(1);
	}

	// Merging thread end.
	err = pthread_join(merging_thread, &output);
	if (err) {
		printf("Error: thread join failed!\n");
		exit(1);
	}

	// Print the result	(Output)
	printf("The array after sorting: \n");
	for (int i = 0; i < n; ++ i)
		printf("%d ", res[i]);
	printf("\n");
	
	// Release the spaces
	free(arr);
	free(res);
	
	return 0;
}
