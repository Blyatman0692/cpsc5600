import java.util.concurrent.CyclicBarrier;

class SortThread implements Runnable {
    public SortThread(int id, int n, int [] data, CyclicBarrier barrier) {
        this.id = id;
        this.n = n;
        this.data = data;
        this.barrier = barrier;
    }

    public void run() {
        // block size
        for (int k = 2; k <= n; k *= 2) {
            // distance between the two elements being compared
            for (int j = k / 2; j > 0; j /= 2) {

                // iterate through the element inside the block
                for (int i = 0; i < n; i++) {

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
            }
        }
    }

    private final int [] data;
    private final int n;
    private final int id;
    private CyclicBarrier barrier;

    private void swap(int i, int j) {
        int temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }
}