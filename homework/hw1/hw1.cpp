/**
 * @file hw1.cpp
 * @author Junwen Zheng
 * @date Jan 17, 2026
 *
 * @brief Ladner–Fischer prefix-sum implementation using a heap-shaped array.
 *
 * This program implements a two-pass parallel prefix-sum algorithm:
 *  1) Up-sweep (reduction): compute subtree sums for all interior nodes.
 *  2) Down-sweep (distribution): propagate prefix offsets to produce inclusive prefix sums.
 *
 * Extra credit strategy (non-power-of-two input sizes):
 *  - Let the real input length be originalSize.
 *  - Let n be the next power of two >= originalSize.
 *  - Build the conceptual tree with n leaves and (n - 1) interior nodes.
 *  - Treat leaf positions [originalSize, n) as logical zeros.
 *  - When writing the output array, only write indices [0, originalSize) and ignore padded leaves.
 */

#include <future>
#include <vector>
#include <iostream>
using namespace std;
const int N = 100000000;

/**
 * @brief Input/output container type for the scan.
 */
typedef vector<int> Data;

/**
 * @class Heaper
 * @brief Utility base class that maps a conceptual complete binary tree onto array indices.
 */
class Heaper {
public:
    /**
     * @brief Construct heap/tree helpers for the provided input.
     *
     * Computes @ref n as the next power of two >= input size and allocates interior storage
     * for (n-1) subtree sums.
     *
     * @param data Pointer to the input array.
     */
    Heaper(const Data* data): originalSize(static_cast<int>(data->size())), data(data) {
        // calculate the next power of 2 number for original array size
        this->n = nextPowerOf2(originalSize);

        interior = new Data(n - 1, 0);
    }

    /** @brief Release interior-node storage. */
    virtual ~Heaper() {
        delete interior;
    }

protected:
    /** @brief Real input length. Only indices [0, originalSize) contain real data. */
    int originalSize;
    /** @brief Tree leaf count. This is the next power of two >= originalSize. */
    int n;
    /** @brief Pointer to the caller-owned input data (leaves). */
    const Data* data;
    /** @brief Storage for interior-node subtree sums; size is (n-1). */
    Data* interior;

    /**
     * @brief Total number of nodes in the conceptual tree (interior + leaves).
     * @return (n-1) + n == 2n-1.
     */
    virtual int size() {
        return n - 1 + n;
    }

    /**
     * @brief Read the value associated with a tree node.
     *
     * For interior nodes, returns the computed subtree sum stored in @ref interior.
     * For leaf nodes, returns the corresponding input value when within bounds, otherwise
     * returns 0.
     *
     * @param i Node index in the heap layout.
     * @return Node value (subtree sum for interior nodes; input or 0 for leaves).
     */
    virtual int value(int i) {
        // for interior nodes
        if (i < n - 1) {
            return interior->at(i);
        }

        // for leaf nodes
        const int k = i - (n - 1);
        // if index < original size, return real data
        if (k < originalSize) {
            return data->at(k);
        }
        // else return logical padding of 0s
        return 0;
    }

    virtual int parent(int i) {
        return (i - 1) / 2;
    }

    virtual int left(int i) {
        return (2 * i) + 1;
    }

    virtual int right(int i) {
        return (2 * i) + 2;
    }

    virtual bool isLeaf(int i) {
        return right(i) >= (n - 1) + n;
    }

    /**
     * @brief Compute the smallest power of two >= n.
     *
     * Used to round an arbitrary input size up to a leaf count suitable for a complete
     * binary tree.
     *
     * @param n Input size.
     * @return Smallest power of two greater than or equal to n.
     */
    static int nextPowerOf2(int n) {
        if (n <= 1) return 1;

        int x = n - 1;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        return x + 1;
    }

};

/**
 * @class SumHeap
 * @brief Implements Ladner–Fischer prefix sums over the heap-shaped tree.
 *
 * Construction performs the up-sweep pass to fill interior subtree sums.
 * The @ref prefixSums method performs the down-sweep pass to produce an
 * inclusive prefix sum array.
 */
class SumHeap: public Heaper {
public:
    /**
     * @brief Construct the heap and compute all interior subtree sums (up-sweep).
     * @param data Pointer to the input array (caller-owned).
     */
    SumHeap(const Data* data): Heaper(data) {
        calcSum(0);
    }

    /**
     * @brief Compute inclusive prefix sums into the provided output vector.
     *
     * Performs the down-sweep pass. For extra-credit padding, only indices [0, originalSize)
     * are written.
     *
     * @param output Pointer to caller-owned output array.
     */
    void prefixSums(Data* output) {
        calcPrefixSums(0, 0, 0, output);
    }


private:
    /**
     * @brief Entry point to recursively compute subtree sums for interior nodes.
     *
     * @param i Node index -- always starts at 0.
     */
    void calcSum(int i) {
        return calcSumHelper(i, 0);
    }

    /**
     * @brief Recursively compute subtree sums for interior nodes.
     *
     * With parallelism the top levels fork tasks; this routine preserves the
     * dependency that a parent sum is computed only after both children are complete.
     *
     * @param i Node index.
     * @param currLevel Node i's current level -- always starts at 0 for root node.
     */
    void calcSumHelper(int i, int currLevel) {
        // base case
        if (isLeaf(i)) {
            return;
        }

        const int leftChild = left(i);
        const int rightChild = right(i);

        // for the first 4 levels, fork a thread
        if (currLevel < 4) {
            auto handle = async(
                launch::async,
                &SumHeap::calcSumHelper,
                this,
                leftChild,
                currLevel + 1
                );

            calcSumHelper(rightChild, currLevel + 1);

            handle.wait();
            interior->at(i) = value(leftChild) + value(rightChild);
        }
        // for the lower levels, do it in the main thread
        else {
            calcSumHelper(leftChild, currLevel + 1);
            calcSumHelper(rightChild, currLevel + 1);

            interior->at(i) = value(leftChild) + value(rightChild);
        }
    }

    /**
     * @brief Recursively propagate prefix offsets and write inclusive scan results.
     *
     * The parameter priorSum represents the sum of all elements strictly before the subtree
     * rooted at node i. The left child inherits priorSum; the right child receives
     * priorSum + (sum of left subtree).
     *
     * Base case writes the final inclusive prefix sum for a leaf into output[k], where
     * k = i - (n-1), when k is within [0, originalSize).
     *
     * @param i Node index.
     * @param priorSum Sum of elements before this subtree.
     * @param output Output array to fill.
     */
    void calcPrefixSums(int i, int priorSum, int currLevel, Data* output) {
        if (isLeaf(i)) {
            const int k = i - (n - 1);

            // if index k is within bounds of original input array, write to
            // corresponding output array
            if (k < originalSize) {
                output->at(k) = priorSum + value(i);
                return;
            }

            // else do nothing
            return;
        }

        const int leftChild = left(i);
        const int rightChild = right(i);

        // for the first 4 levels, fork a thread
        if (currLevel < 4) {
            auto handle = async(
                launch::async,
                &SumHeap::calcPrefixSums,
                this,
                leftChild,
                priorSum,
                currLevel + 1,
                output
                );

            calcPrefixSums(rightChild, priorSum + value(leftChild),
                currLevel + 1, output);
            handle.wait();
        }
        // for the lower levels, do it in the main thread
        else {
            calcPrefixSums(leftChild, priorSum, currLevel + 1, output);
            calcPrefixSums(rightChild, priorSum + value(leftChild),
                currLevel + 1, output);
        }
    }
};

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