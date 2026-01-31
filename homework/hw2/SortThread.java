import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
/**
 * @author: Junwen Zheng
 * @date: Jan 25, 2026
 * @file: SortThread.java
 *
 * This java files contains class SortThread that implements a parallel version of Bitonic loop sort.
 * Each instance is responsible for a contiguous partition of the shared array. Each thread synchronizes
 * with peers using a CyclicBarrier object, where the execution of the next critical section is blocked
 * until all threads are ready to enter the section.
 */

/**
 * A worker thread that executes the bitonic sorting network on a shared array for a fixed index partition.
 *
 * <p>All workers run the same loop schedule. The only difference is the range of indices
 * each worker processes in the innermost loop. After completing one network column
 * (one iteration of j), each worker waits at a shared {@link CyclicBarrier} so the next column
 * processes a consistent array state.
 */
class SortThread implements Runnable {
    /**
     * Creates a worker for a specific partition of the array.
     *
     * @param id      worker id.
     * @param n       total array size; must be a power of two for bitonic sort.
     * @param section partition size, assumes n fully divisible by p.
     * @param data    shared array to be sorted (all threads reference the same array).
     * @param barrier shared barrier used to synchronize after each network column.
     */
    public SortThread(int id, int n, int section, int [] data, CyclicBarrier barrier) {
        this.id = id;
        this.n = n;
        this.data = data;
        this.barrier = barrier;

        start = id * section;
        end = start + section;
    }

    /**
     * Executes the loop-based bitonic sort over this thread's assigned index range.
     *
     * <p>Loop iterations:
     * <ul>
     *   <li>{@code k}: current stage/block size being merged (2, 4, 8, ..., n).</li>
     *   <li>{@code j}: compare distance for the current column within stage {@code k} (k/2, k/4, ..., 1).</li>
     *   <li>{@code i}: a "wire" index; this worker processes {@code i} only in {@code [start, end)}.</li>
     * </ul>
     *
     * <p> Sort direction:
     * <ul>
     *   <li>If {@code (i & k) == 0}, enforce ascending order within the current {@code k}-block.</li>
     *   <li>Otherwise, enforce descending order.</li>
     * </ul>
     *
     * <p>Synchronization:
     * After completing all comparisons for a given {@code (k, j)} column over this thread's wires,
     * the thread waits at the barrier so that no thread advances to the next column early.
     */
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

    /**
     * Swaps two elements in the shared array.
     *
     * @param i first index
     * @param j second index
     */
    private void swap(int i, int j) {
        int temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }
}