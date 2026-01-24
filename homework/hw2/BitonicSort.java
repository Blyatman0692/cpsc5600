import java.util.Random;
import java.util.concurrent.CyclicBarrier;

public class BitonicSort{
    static public void fillWithRandomNum(int[] data) {
        Random rand = new Random();
        for (int i = 0; i < data.length; i++) {
            data[i] = rand.nextInt();
        }
    }

    public static void main(String[] args) {
        int p = Integer.parseInt(args[0]);
        int granularity = Integer.parseInt(args[1]);
        int n = 1 << 3;  // N = 2^22
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
                threads[i] = new Thread(new SortThread(i, n, section, data, barrier));
                threads[i].start();
            }

            for (Thread t : threads) {
                try {
                    t.join();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            // TODO: (optional) verify it's sorted
            count++;
        }

        long elapsed = System.currentTimeMillis() - start;
        System.out.println("Sorted " + count + " arrays (each: " + n + " elements) in " +
                elapsed + " ms using " + p + " threads");
    }
}

