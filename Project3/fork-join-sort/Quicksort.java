import java.util.Arrays;
import java.util.Scanner;
import java.util.concurrent.*;

public class Quicksort extends RecursiveAction {
	static final int THRESHOLD = 10;

	private int begin;
	private int end;
	private int[] array;

	public Quicksort(int begin, int end, int[] array) {
		this.begin = begin;
		this.end = end;
		this.array = array;
	}

	protected void compute() {
		if (end - begin + 1 <= THRESHOLD) {
			// Sort directly using Bubble Sort
			for (int i = end; i >= begin + 1; -- i)
				for (int j = begin; j < i; ++ j)
					if (array[j] > array[j + 1]) {
						int temp = array[j];
						array[j] = array[j + 1];
						array[j + 1] = temp;
					}
		} else {
			// divide stage 
			int pivot = array[begin];
			int low = begin, high = end;
			while (low < high) {
				while (low < high && array[high] >= pivot) -- high;
				if (low < high) array[low ++] = array[high];
				while (low < high && array[low] <= pivot) ++ low;
				if (low < high) array[high --] = array[low];
			}
			array[low] = pivot;
            
			Quicksort leftTask = new Quicksort(begin, low - 1, array);
			Quicksort rightTask = new Quicksort(low + 1, end, array);

			leftTask.fork();
			rightTask.fork();

			leftTask.join();
			rightTask.join();
		}
	}

	public static void main(String[] args) {
		ForkJoinPool pool = new ForkJoinPool();
		Scanner sc = new Scanner(System.in);

		System.out.print("Please input the length of the array (0 <= n <= 1000000): ");
		int n = sc.nextInt();
		if (n < 0 || n > 1000000) {
			System.out.println("Error: n should be in range [0, 1000000]!");
			System.exit(1);
		}

		int[] array = new int[n];

		System.out.print("Do you want to generate the random elements automatically (y/n): ");
		char opt = sc.next().charAt(0);
		if (opt == 'y') {
			// create SIZE random integers between 0 and 999
			java.util.Random rand = new java.util.Random();
			for (int i = 0; i < n; ++ i) 
				array[i] = rand.nextInt(1000);
			System.out.print("The original array: ");
			System.out.println(Arrays.toString(array));
		} else if (opt == 'n') {
			System.out.println("Please input the array elements: ");
			for (int i = 0; i < n; ++ i)
				array[i] = sc.nextInt();
		} else {
			System.out.println("Error: invalid input!");
			System.exit(1);
		}
		
		// use fork-join parallelism to sum the array
		Quicksort task = new Quicksort(0, n - 1, array);

		pool.invoke(task);

		System.out.print("The array after sorting: ");
		System.out.println(Arrays.toString(array));
	}
}
