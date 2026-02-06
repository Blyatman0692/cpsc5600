/**
 * CPSC 5600, Quiz 4, Question 3 scaffolding
 * Seattle U, Winter 2026
 * Kevin Lundeen
 */

#include <iostream>
#include <cstring>
#include "mpi.h"

using namespace std;

class Quiz4Question3 {
public:
    /**
     * Set up an instance with the data shown on the quiz example
     * then set up the preconditions for scatterOverlap, call it, and then print out the
     * m and partition which are promised by scatterOverlap postconditions.
     */
    static void test() {
        u_int tdata[] = {1, 12,3, 80, 200, 12, 3,
                         14, 91, 0, 4, 200, 5, 90,
                         634, 876, 12, 6, 7, 1, 10};
        Quiz4Question3 t;
        MPI_Comm_size(MPI_COMM_WORLD, &t.p);
        MPI_Comm_rank(MPI_COMM_WORLD, &t.rank);
        if (t.rank == t.ROOT) {
            t.n = sizeof(tdata) / sizeof(tdata[0]);
            t.data = tdata;
        }

        // send n to everyone
        MPI_Bcast(&t.n, 1, MPI_INT, t.ROOT, MPI_COMM_WORLD);

        // test scatterBuckets
        t.scatterOverlap();
        cout << t.rank << " has data (size: " << t.m << "): { ";
        for (int i = 0; i < t.m; i++)
            cout << t.partition[i] << " ";
        cout << "}" << endl;
    }

private:
    int n = 0;                         // number of elements (no padding)
    unsigned int *data = nullptr;      // data array
    int p = 1;                         // size of MPI_COMM_WORLD
    const int ROOT = 0;                // rank of root process in MPI_COMM_WORLD
    const int OVERLAP = 2;             // number of elements to overlap on each side
    int rank = ROOT;                   // this process' rank in MPI_COMM_WORLD
    unsigned int *partition = nullptr; // per-process section of data (plus padding on ends)
    int m = 0;                         // size of partition


    /**
     * @pre n, p are set for all; data is set for ROOT only
     * @post m, partition are set including the overlap into previous/next sections
     */
    void scatterOverlap() {
        u_int *sendbuf = nullptr;
        int *sendcounts = nullptr, *displs = nullptr;

        if (rank == ROOT) {
            // marshal data into sendbuf and set up sending side of message (ROOT only)
            // FIXME
        }

        // set this->m for my process
        m = 0; // FIXME

        // set up receiving side of message (everyone)
        partition = new u_int[m];

        MPI_Scatterv(sendbuf, sendcounts, displs, MPI_UNSIGNED,
                     partition, m, MPI_UNSIGNED,
                     ROOT, MPI_COMM_WORLD);

        // no unmarshalling--what's sent is already in the form we want

        // free temp arrays
        delete[] sendbuf;
        delete[] sendcounts;
        delete[] displs;
    }
};

int main() {
    MPI_Init(nullptr, nullptr);
    Quiz4Question3::test();
    MPI_Finalize();
}
