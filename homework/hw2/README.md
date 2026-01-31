In this exercise, you will parallelize the bitonic sort algorithm using Java threads and barrier synchronization. Each thread will be responsible for a subset of the array. Using our network diagram as an analogy, each thread will be responsible for certain "wires" and will perform all the comparators that initiate on its wires (the arrows on the diagram). Synchronization will be done when threads need to compare/swap with other threads' wires, using the Java class CyclicBarrier Links to an external site. to coordinate the threads. [You may use Phaser Links to an external site. object only if you do the extra credit--this has a neat built-in support for hierarchical barriers which could help you keep many different barriers organized.]

AI context: In large language models, sorting operations appear in beam search Links to an external site. (ranking candidate sequences by probability) and top-k/top-p sampling Links to an external site. (selecting the most likely next tokens). Bitonic sort's fixed, data-independent communication pattern makes it well-suited for GPU implementation where predictable memory access is crucial. The barrier synchronization pattern you'll use here—where all threads must complete one phase before any can proceed to the next—is fundamental to parallel algorithms on both CPUs and GPUs.

Java Concurrency Primer
This assignment uses Java threads and barriers. Here's what you need to know:

Creating Threads with Runnable
class MyWorker implements Runnable {
private int id;

    public MyWorker(int id) {
        this.id = id;
    }
    
    @Override
    public void run() {
        // do work here
    }
}

// To start threads:
Thread[] threads = new Thread[P];
for (int i = 0; i < P; i++) {
threads[i] = new Thread(new MyWorker(i));
threads[i].start();
}

// To wait for all threads to finish:
for (Thread t : threads) {
t.join();
}
CyclicBarrier
A CyclicBarrier allows a set of threads to wait for each other to reach a common point. It's "cyclic" because it can be reused after the waiting threads are released.

// Create a barrier for P threads
CyclicBarrier barrier = new CyclicBarrier(P);

// In each thread's run() method, wait at the barrier:
barrier.await();  // blocks until all P threads have called await()
For more details, see the Java Concurrency Tutorial Links to an external site..

The Algorithm
Model this approach with bitonic_loops.cpp Download bitonic_loops.cpp, which moves k forward through all the stages, then for each k, j moves forward through each column, then for each j, we look at each wire, i, and do the compare/swap with its partner, ixj. (There is a small optimization in bitonic_loops.cpp that skips the compare/swap if it has already been done for the same i vs. ixj pair.)

Requirements
This exercise will be written in Java and attempt to increase throughput by the use of up to 8 symmetrical threads.

Your program should accept two command-line parameters:

P — number of threads (1, 2, 4, or 8)
GRANULARITY — barrier granularity level:
GRANULARITY=1 — one barrier for all threads after each column (i.e., for each value of j). This is the "large granularity" approach and is the easiest to implement.
Higher values of GRANULARITY (extra credit) — more fine-grained synchronization, potentially improving performance by reducing unnecessary waiting. The value itself should denote the maximum number of barriers at any boundary, e.g., GRANULARITY=4 would mean that at some time(s) there are four different barriers in place on a single boundary--red line in the diagram.
Start, as always, with small values. Set N=16, and P=2 with GRANULARITY=1. Once that is working, bump up N to 222 and P to 8.

Test your results by comparing P=1, P=2, P=4, and P=8 (all with GRANULARITY=1). You should see substantial improvements in throughput. Note that this program is not pipelined at all. We start with a random array and completely sort it using all our threads in unison. Then we sort another one, etc. The throughput is measured by seeing how many arrays of a given size, N, can be sorted in, say, 10 seconds.

Throughput Measurement
Your main program should measure throughput by sorting multiple random arrays in a fixed time window. Here's the general structure:

public static void main(String[] args) {
int p = Integer.parseInt(args[0]);
int granularity = Integer.parseInt(args[1]);
int n = 1 << 22;  // N = 2^22
double seconds = 10.0;

    // TODO: setup your sorter with n, p, and granularity
    
    int count = 0;
    long start = System.currentTimeMillis();
    long end = start + (long)(seconds * 1000);
    
    while (System.currentTimeMillis() < end) {
        // TODO: fill array with random data
        // TODO: sort the array
        // TODO: (optional) verify it's sorted
        count++;
    }
    
    long elapsed = System.currentTimeMillis() - start;
    System.out.println("Sorted " + count + " arrays (each: " + n + " elements) in " + 
                       elapsed + " ms using " + p + " threads");
}
Run your program with P=1, P=2, P=4, and P=8 (all with GRANULARITY=1) and compare the throughput (arrays sorted per second). Include these results in a comment at the top of your main class.

Guidance
Each thread should be running the exact same code, but differ only in which values of i in the innermost loop it is responsible for. For instance, if N=16 and P=4, we'd have the following thread allocation:

bitonic sync thread layout

Of course, it cannot do its compare/swaps until the wires it is comparing to (its values of ixj) are already finished from the previous column (the previous value of j). That's where you need to install the barriers. The easiest way is to install a barrier for all the threads after each column. For instance, if N=16, with max granularity, you'd have barriers as follows:

barriers

The CyclicBarrier class in Java is just a class that allows you to re-use the barrier objects over and over instead of reconstructing them each time (a better name would have been ReusableBarrier).

Submission
Submit a zip file containing your Java source files. Use the following class names:

BitonicSort.java — your main class with the main method and throughput measurement
SortThread.java — your Runnable class for the worker threads (or you may include this as an inner class in BitonicSort)
Include a comment at the top of BitonicSort.java with your throughput results for P=1, P=2, P=4, and P=8 (all with GRANULARITY=1).

Your program should compile and run on CS1 with:

$ javac *.java
$ java BitonicSort 8 1
(where 8 is P and 1 is GRANULARITY)