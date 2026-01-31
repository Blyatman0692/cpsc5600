import java.util.Random;
import java.util.concurrent.CyclicBarrier;

/**
 * @author: Junwen Zheng
 * @date: Jan 25, 2026
 * @file: BitonicSort.java
 *
 * <p>Runs the loop-based (sorting-network) bitonic sort using {@link SortThread} workers.
 * Each worker owns a contiguous slice of a shared {@code int[]} and syncs after each network column
 * using a {@link CyclicBarrier} (granularity = 1).
 *
 * <p>Throughput results (GRANULARITY=1):
 * <ul>
 *   <li>P=1: Sorted 6 arrays (each: 4194304 elements) in 10121 ms</li>
 *   <li>P=2: Sorted 12 arrays (each: 4194304 elements) in 10266 ms</li>
 *   <li>P=4: Sorted 22 arrays (each: 4194304 elements) in 10315 ms</li>
 *   <li>P=8: Sorted 36 arrays (each: 4194304 elements) in 10065 ms</li>
 * </ul>
 *
 * <ul>
 *   <li>Assumes {@code n} and {@code p} are power of two.</li>
 *   <li>Additional finding: throughput peaked at {@code p = 8} due to overhead.</li>
 * </ul>
 */
public class BitonicSort{
    /** Fills the array with random 32-bit integers. */
    static public void fillWithRandomNum(int[] data) {
        Random rand = new Random();
        for (int i = 0; i < data.length; i++) {
            data[i] = rand.nextInt();
        }
    }

    /** Returns true if the array is sorted in nondecreasing order. */
    static public boolean verifySorted(int [] bitonicSorted) {
        for (int i = 1; i < bitonicSorted.length; i++) {
            if (bitonicSorted[i] < bitonicSorted[i - 1]) {
                return false;
            }
        }
        return true;
    }

    /**
     * Entry point.
     *
     * <p>Args:
     * <ul>
     *   <li>{@code args[0]}: P (number of threads: power of 2)</li>
     *   <li>{@code args[1]}: GRANULARITY (input only 1)</li>
     * </ul>
     */
    public static void main(String[] args) {
        int p = Integer.parseInt(args[0]);
        int granularity = Integer.parseInt(args[1]);
        int n = 1 << 22;  // N = 2^22
        double seconds = 10.0;

        int section = n / p ;
        System.out.println("Each thread processes: " + section + " data");
        int [] data = new int[n];
        Thread[] threads = new Thread[p];
        CyclicBarrier barrier = new CyclicBarrier(p);

        int count = 0;
        long start = System.currentTimeMillis();
        long end = start + (long)(seconds * 1000);

        while (System.currentTimeMillis() < end) {
            // fill array with random data
            fillWithRandomNum(data);
            // sort the array
            for (int i = 0; i < p; i++) {
                // initialize each thread and start
                threads[i] = new Thread(new SortThread(i, n, section, data, barrier));
                threads[i].start();
            }
            // wait for all threads to finish
            for (Thread t : threads) {
                try {
                    t.join();
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }
            // (optional) verify it's sorted
            if (verifySorted(data)) {
                System.out.println(count + " verified to be sorted");
            } else {
                System.out.println(count + " sort failed");
            }

            count++;
        }

        long elapsed = System.currentTimeMillis() - start;
        System.out.println("Sorted " + count + " arrays (each: " + n + " elements) in " +
                elapsed + " ms using " + p + " threads");
    }
}
