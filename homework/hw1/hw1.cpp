//
// Created by Junwen Zheng on 1/11/26.
//

#include <future>
#include <vector>
#include <iostream>
using namespace std;
const int N = 2<<26;  // FIXME must be power of 2 for now

typedef vector<int> Data;

class Heaper {
public:
    Heaper(const Data* data): n(static_cast<int>(data->size())), data(data) {
        interior = new Data(n - 1, 0);
    }

    ~Heaper() {
        delete interior;
    }

protected:
    int n;
    const Data* data;
    Data* interior;

    virtual int size() {
        return n - 1 + n;
    }

    virtual int value(int i) {
        if (i < n - 1) {
            return interior->at(i);
        }
        return data->at(i - (n - 1));
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
        if (i < n - 1) {
            return false;
        }

        return true;
    }

};

class SumHeap: public Heaper {
public:
    SumHeap(const Data* data): Heaper(data) {
        calcSum(0);
    }

    void prefixSums(Data* output) {
        prefixSums(0, 0, output);
    }


private:
    void calcSum(int i) {
        // base case
        if (isLeaf(i)) {
            return;
        }

        calcSum(left(i));
        calcSum(right(i));

        interior->at(i) = value(left(i)) + value(right(i));
    }

    void prefixSums(int i, int priorSum, Data* output) {
        if (isLeaf(i)) {
            output->at(i - (n - 1)) = priorSum + value(i);
            return;
        }

        const int leftChild = left(i);
        const int rightChild = right(i);

        prefixSums(leftChild, priorSum, output);
        prefixSums(rightChild, priorSum + value(leftChild), output);
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