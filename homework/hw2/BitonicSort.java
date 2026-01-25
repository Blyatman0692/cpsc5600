import java.util.Random;
import java.util.concurrent.CyclicBarrier;

public class BitonicSort{
    static public void fillWithRandomNum(int[] data) {
        Random rand = new Random();
        for (int i = 0; i < data.length; i++) {
            data[i] = rand.nextInt();
        }
    }

    static public boolean verifySorted(int [] bitonicSorted) {
        for (int i = 1; i < bitonicSorted.length; i++) {
            if (bitonicSorted[i] < bitonicSorted[i - 1]) {
                return false;
            }

        }
        return true;
    }

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

