import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

class SortThread implements Runnable {
    public SortThread(int id, int n, int section, int [] data, CyclicBarrier barrier) {
        this.id = id;
        this.n = n;
        this.data = data;
        this.barrier = barrier;

        start = id * section;
        end = start + section;
    }

    public void run() {
        // block size
        for (int k = 2; k <= n; k *= 2) {
            // distance between the two elements being compared
            for (int j = k / 2; j > 0; j /= 2) {
                // iterate through the element inside the block
                for (int i = start; i < end; i++) {
                    // partner index
                    int ixj = i ^ j;

                    if (ixj > i) {
                        if ((i & k) == 0 && data[i] > data[ixj]) {
                            swap(i, ixj);
                        }
                        if ((i & k) != 0 && data[i] < data[ixj]) {
                            swap(i, ixj);
                        }
                    }
                }

                // try-catch block to wait for all threads
                try {
                     // System.out.println("Thread:" + id + " waiting.");
                    barrier.await();
                    // System.out.println("Thread:" + id + " passed.");
                } catch (BrokenBarrierException | InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }
        }
    }

    private final int [] data;
    private final int n;
    private final int id;
    private final int start;
    private final int end;
    private final CyclicBarrier barrier;

    private void swap(int i, int j) {
        int temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }
}