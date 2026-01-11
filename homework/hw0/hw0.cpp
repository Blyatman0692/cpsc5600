/**
 * Based on idea from Matthew Flatt, Univ of Utah
 * Junwen Zheng
 * Marcel Nicasio
 * HW0 Group 8
 * Jan 9, 2026
 */
#include <iostream>
#include "ThreadGroup.h"
using namespace std;

int encode(int v) {
	// do something time-consuming (and arbitrary)
	for (int i = 0; i < 500; i++)
		v = ((v * v) + v) % 10;
	return v;
}

int decode(int v) {
	// do something time-consuming (and arbitrary)
	return encode(v);
}

/**
 * @struct SharedData: the data structure shared between threads which contains
 * length of the array used to split work between threads (given by prefixSum),
 * and the pointer to the actual array
 */
struct SharedData {
	int length = 0;
	int* data = nullptr;
};

/**
 * @class EncodeThread: the template argument to ThreadGroup which has the
 * implementation of ()-operator.
 */
class EncodeThread {
public:
	void operator()(int id, void* sharedData) {
		// Cast void* to SharedData data type
		const auto *shared = static_cast<SharedData*>(sharedData);

		// Calculate the section each thread is responsible for using the length
		// given in ShardData
		const int length_per_thread = shared->length / 2;
		const int start = id * length_per_thread;

		// For each element in the responsible section encode the element
		for (int i = start; i < start + length_per_thread; i++) {
			shared->data[i] = encode(shared->data[i]);
		}

		cout << "Encoder " << id << " finished" << endl;
	}
};

/**
 * @class DecodeThread: the template argument to ThreadGroup which has the
 * implementation of ()-operator.
 */
class DecodeThread {
public:
	void operator()(int id, void* sharedData) {
		// Cast void* to SharedData data type
		const auto *shared = static_cast<SharedData*>(sharedData);

		// Calculate the section each thread is responsible for using the length
		// given in ShardData
		const int length_per_thread = shared->length / 2;
		const int start = id * length_per_thread;

		// For each element in the responsible section decode the element
		for (int i = start; i < start + length_per_thread; i++) {
			shared->data[i] = decode(shared->data[i]);
		}

		cout << "Decoder " << id << " finished" << endl;
	}
};

void prefixSums(int *data, int length) {
	int encodedSum = 0;
	SharedData sharedData;
	sharedData.length = length;
	sharedData.data = data;

	// Encoding threads
	ThreadGroup<EncodeThread> encoders;
	encoders.createThread(0, &sharedData);
	encoders.createThread(1, &sharedData);
	encoders.waitForAll();

	// Main thread doing prefix sums
	for (int i = 0; i < length; i++) {
		encodedSum += data[i];
		data[i] = encodedSum;
	}

	// Decoding threads
	ThreadGroup<DecodeThread> decoders;
	decoders.createThread(0, &sharedData);
	decoders.createThread(1, &sharedData);
	decoders.waitForAll();
}

int main() {
	int length = 1000 * 1000;

	// make array
	int *data = new int[length];
	for (int i = 1; i < length; i++)
		data[i] = 1;
	data[0] = 6;

	// transform array into converted/deconverted prefix sum of original
	prefixSums(data, length);

	// printed out result is 6, 6, and 2 when data[0] is 6 to start and the rest 1
	cout << "[0]: " << data[0] << endl
			<< "[" << length/2 << "]: " << data[length/2] << endl
			<< "[end]: " << data[length-1] << endl;

	delete[] data;
	return 0;
}