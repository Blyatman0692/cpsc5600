In this exercise, you will produce an implementation of the Ladner-Fischer† parallel prefix sum algorithm using the C++ thread library Links to an external site.. The interior nodes of the tree in the algorithm will be implemented using an array (like a heap). Write a Heaper base class as shown in the lecture slides and extend that for your own SumHeap class. The data array will be in a std::vector<int> typedef'd to be called Data.

†The Ladner-Fischer algorithm Links to an external site. is the one discussed in lecture in week 1 and week 2.

AI context: The parallel prefix sum you're implementing is a fundamental building block in neural network computation. When a transformer computes softmax Links to an external site. (used in every attention layer), it sums exponentials across thousands of elements—that's a parallel reduction, the first pass of this algorithm. The same up-sweep/down-sweep pattern appears in optimized GPU kernels for LLM training and inference.
You are allowed to make the number of elements in the array constrained to be a power of 2. There is extra credit for relaxing this requirement such that the algorithm still works correctly.

The recursive adder pass must be parallelized for the top four levels of the tree by forking a thread, using std::async, at each of those levels but done within the same thread for levels below that (total of 16 threads). Similarly, the prefix summation pass must also be done in parallel in the same fashion.

In your testing, use smaller arrays, of course, but for your final submission use an array of 67,108,864 elements (226) which on CS1 should run in about 600ms without optimization (with the -O2 compiler flag, my 16-thread solution runs in 100ms). If you are handing in a submission that includes the extra credit, then make your data array 100,000,000 elements and note that you are turning in the extra credit and what your strategy is.

Your program should all be in a single file, hw1.cpp.

It should include a test outside the timing loop. To satisfy this requirement, you may use the following code verbatim:

const int N = 1<<26;  // FIXME must be power of 2 for now

int main() {
    Data data(N, 1);  // put a 1 in each element of the data array
    data[0] = 10;
    Data prefix(N, 1);

    // start timer
    auto start = chrono::steady_clock::now();

    SumHeap heap(&data);
    heap.prefixSums(&prefix);

    // stop timer
    auto end = chrono::steady_clock::now();
    auto elpased = chrono::duration<double,milli>(end-start).count();

    int check = 10;
    for (int elem: prefix)
        if (elem != check++) {
            cout << "FAILED RESULT at " << check-1;
            break;
        }
    cout << "in " << elpased << "ms" << endl;
    return 0;
}
Guidance
Passing methods to async
For the thread forks in your recursive routines, you will likely want to call std::async with a method instead of a function as demonstrated in the example programs. To do this, since it is not a static method, you need to pass in this as the first argument. My call to async in my recursive method calcSum, looks like this, for instance:

auto handle = async(launch::async, &SumHeap::calcSum, this, left(i), level+1);
Thread Allocations
You may want to keep track of the level of the recursion by adding in a parameter to your calcSum and calcPrefix methods so they know when to stop forking threads. Alternatively, you could write a level(i) method for your Heaper class.

Two-Pass Algorithm
The prefix sum algorithm is a two-pass algorithm.

The pair-wise sum pass. This is done during the construction of the SumHeap object and must be done in parallel (using 16 threads as noted above).
SumHeap heap(&data);
The prefix sum pass. This is done within the prefixSums method and must also be done in parallel. The base case of this recursion writes the prefix sum values into the output array.
heap.prefixSums(&prefix);
valgrind
By the way, the valgrind tool is an excellent way to check that your program has no memory leaks and to track down memory bugs in C++ code. 

$ valgrind ./hw1_setup
==104044== Memcheck, a memory error detector
==104044== Copyright (C) 2002-2024, and GNU GPL'd, by Julian Seward et al.
==104044== Using Valgrind-3.23.0 and LibVEX; rerun with -h for copyright info
==104044== Command: ./hw1_setup
==104044== 
[0]: 6
[500000]: 6
[end]: 2
==104044== 
==104044== HEAP SUMMARY:
==104044==     in use at exit: 0 bytes in 0 blocks
==104044==   total heap usage: 3 allocs, 3 frees, 4,073,728 bytes allocated
==104044== 
==104044== All heap blocks were freed -- no leaks are possible
==104044== 
==104044== For lists of detected and suppressed errors, rerun with: -s
==104044== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
Version
Last updated: 6-Jan-2025
