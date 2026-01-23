import java.util.ArrayList;
import java.util.Random;

class SortThread implements Runnable {
    public SortThread(int p, int n) {
        this.n = n;
        data = new int[n];
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

    private void swap(int i, int j) {
        int temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }

    public void fillWithRandomNum(int n) {
        Random rand = new Random();
        for (int i = 0; i < n; i++) {
            data[i] = rand.nextInt();
        }
    }
}