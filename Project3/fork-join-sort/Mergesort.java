import java.util.Arrays;
import java.util.Scanner;
import java.util.concurrent.*;

public class Mergesort extends RecursiveAction {
	static final int THRESHOLD = 10;

	private int begin;
	private int end;
	private Integer[] array;

	public Mergesort(int begin, int end, Integer[] array) {
		this.begin = begin;
		this.end = end;
		this.array = array;
	}

	protected void compute() {
		if (end - begin + 1 <= THRESHOLD) {
			// Sort directly using Bubble Sort
			for (int i = end; i >= begin + 1; -- i)
				for (int j = begin; j < i; ++ j)
					if (array[j].compareTo(array[j + 1]) > 0) {
						Integer temp = array[j];
						array[j] = array[j + 1];
						array[j + 1] = temp;
					}
		} else {
			// divide stage 
			int mid = begin + (end - begin) / 2;
            
			Mergesort leftTask = new Mergesort(begin, mid, array);
			Mergesort rightTask = new Mergesort(mid + 1, end, array);

			leftTask.fork();
			rightTask.fork();

			leftTask.join();
			rightTask.join();

			Integer[] temp = new Integer [end - begin + 1];
			
			int pos1 = begin, pos2 = mid + 1, k = 0;
			while (pos1 <= mid && pos2 <= end) {
				if (array[pos1].compareTo(array[pos2]) <= 0) temp[k ++] = array[pos1 ++];
				else temp[k ++] = array[pos2 ++];
			}
			while (pos1 <= mid) temp[k ++] = array[pos1 ++];
			while (pos2 <= end) temp[k ++] = array[pos2 ++];

			for (int i = 0; i < k; ++ i)
				array[i + begin] = temp[i];
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

		Integer[] array = new Integer[n];

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
		Mergesort task = new Mergesort(0, n - 1, array);

		pool.invoke(task);

		System.out.print("The array after sorting: ");
		System.out.println(Arrays.toString(array));
	}
}
