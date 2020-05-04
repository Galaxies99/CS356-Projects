// Note: remember to add '-g -lpthread' when compiling.
# include <stdio.h>
# include <pthread.h>
# include <stdlib.h>

int n;
int *arr;

struct array_parameter {
	int n;
	int *arr;
};

struct merge_parameter {
  struct array_parameter *arg[2];
	struct array_parameter *res;
};

int qsort_cmp(const void *a, const void *b) {
	return * (int *) a - * (int *) b;
}

void* sorting_routine(void *arg) {
	struct array_parameter *data = (struct array_parameter *) arg;

	// Quick sort provided by <stdlib.h>
	qsort(data -> arr, data -> n, sizeof(int), qsort_cmp);
	
	// return
  void *ret = data;
  return ret;
}

void* merging_routine(void *arg) {
	struct merge_parameter *data = (struct merge_parameter *) arg;
	
	// Allocate result spaces.
	data -> res = (struct array_parameter *) malloc (sizeof(struct array_parameter));
	data -> res -> n = data -> arg[0] -> n + data -> arg[1] -> n;
	data -> res -> arr = (int *) malloc (data -> res -> n * sizeof(int));

	// Merge two sorted arrays.
	// |-- pos of result
	int res_pos = 0;
	// |-- pos of two arrays.
	int pos0 = 0, pos1 = 0;
	// |-- the current elements of two arrays.				
	int element0, element1;
	// |-- whether to fetch the elements (1 - fetch, 0 - not fetch).
	int fetch0 = 1, fetch1 = 1;

	// |-- comparing & merging.
	while (pos0 < data -> arg[0] -> n && pos1 < data -> arg[1] -> n) {
		if (fetch0) element0 = data -> arg[0] -> arr[pos0];
		if (fetch1) element1 = data -> arg[1] -> arr[pos1];
		if (element0 <= element1) {
			data -> res -> arr[res_pos ++] = element0;
			pos0 ++; fetch0 = 1; fetch1 = 0;
		} else{
			data -> res -> arr[res_pos ++] = element1;
			pos1 ++; fetch0 = 0; fetch1 = 1;
		}
	}
	// |-- the rest of the elements.
	while (pos0 < data -> arg[0] -> n)
		data -> res -> arr[res_pos ++] = data -> arg[0] -> arr[pos0 ++];
	while (pos1 < data -> arg[1] -> n)
		data -> res -> arr[res_pos ++] = data -> arg[1] -> arr[pos1 ++];
	
	// Check the result.
	if (res_pos != data -> res -> n) {
		printf("Error: unexpected error occurs when merging.\n");
		exit(1);	
	}
	
	// return;
	void *ret = data;
	return ret;
}
	
int main(void) {
	int err;
	
	// Input
	printf("Please input the number of elements (0 <= n <= 1000000): ");
  scanf("%d", &n);
  
  if(n < 0 || n > 1000000) {
    printf("Error: n should be in range [0, 1000000]!\n");
    exit(1);
  }
  
  arr = (int *) malloc (n * sizeof(int)); 
  
  printf("Please input the elements: \n");
  for (int i = 0; i < n; ++ i)
		scanf("%d", &arr[i]);

	// Prepare parameters for sorting threads.
	struct array_parameter param[2];
	// |-- thread 0 parameters  
	param[0].n = n / 2;
	param[0].arr = arr;
	// param[0].arr = &arr[0];
	// |-- thread 1 parameters
	param[1].n = n - n / 2;
  param[1].arr = &arr[n / 2];
	
	// Create sorting threads & passing parameters.
	pthread_t sorting_thread[2];
	for (int i = 0; i < 2; ++ i) {
  	err = pthread_create(&sorting_thread[i], NULL, sorting_routine, &param[i]);
		if (err) {
			printf("Error: create thread failed!\n");
			exit(1);
		}
	}

	// Receive output from sorting threads & prepare parameters for merging thread.
	void *output;
	struct merge_parameter m_param;
  for (int i = 0; i < 2; ++ i) {
    err = pthread_join(sorting_thread[i], &output);
		if (err) {
			printf("Error: thread join failed!\n");
			exit(1);
		}
		m_param.arg[i] = (struct array_parameter *) output;
	}

	// Create merging thread & passing parameters.
	pthread_t merging_thread;
	err = pthread_create(&merging_thread, NULL, merging_routine, &m_param);
  if (err) {
		printf("Error: create thread failed!\n");
		exit(1);
	}

	// Receive output from merging thread
	err = pthread_join(merging_thread, &output);
	struct merge_parameter *ans = (struct merge_parameter *) output;
	if (err) {
		printf("Error: thread join failed!\n");
		exit(1);
	}
	// Print the result	(Output)
	printf("The array after sorting: \n");
	for (int i = 0; i < ans -> res -> n; ++ i)
		printf("%d ", ans -> res -> arr[i]);
	printf("\n");
	
	// Release the spaces
	free(ans -> res -> arr);
	free(ans -> res);
	free(arr);
	
	return 0;
}
