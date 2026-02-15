CPPFLAGS = -std=c++20 -Wall -Werror -pedantic -ggdb
PROGRAMS = kmean_color_test hw3 emnist

all : $(PROGRAMS)

Color.o : Color.cpp Color.h
	mpic++ $(CPPFLAGS) $< -c -o $@

kmean_color_test.o : kmean_color_test.cpp Color.h ColorKMeans.h KMeans.h
	mpic++ $(CPPFLAGS) $< -c -o $@

kmean_color_test : kmean_color_test.o Color.o
	mpic++ $(CPPFLAGS) kmean_color_test.o Color.o -o $@

run_sequential : kmean_color_test
	./kmean_color_test

hw3.o : hw3.cpp Color.h ColorKMeansMPI.h KMeansMPI.h
	mpic++ $(CPPFLAGS) $< -c -o $@

hw3 : hw3.o Color.o
	mpic++ $(CPPFLAGS) hw3.o Color.o -o $@

run_hw3 : hw3
	mpirun -n 2 ./hw3

valgrind : hw3
	mpirun -n 2 valgrind ./hw3

bigger_test : hw3
	mpirun -n 10 ./hw3

biggest_test : hw3
	mpirun -n 32 ./hw3

# ===== extra credit =====
emnist.o : emnist.cpp IdxIO.h KMeansMPI.h
	mpic++ $(CPPFLAGS) $< -c -o $@

IdxIO.o : IdxIO.cpp IdxIO.h
	mpic++ $(CPPFLAGS) $< -c -o $@

emnist : emnist.o IdxIO.o
	mpic++ $(CPPFLAGS) emnist.o IdxIO.o -o $@

run_emnist : emnist
	mpirun -n 2 ./emnist emnist-digits-train-images-idx3-ubyte emnist-digits-train-labels-idx1-ubyte

bigger_test_emnist : emnist
	mpirun -n 10 ./emnist emnist-digits-train-images-idx3-ubyte emnist-digits-train-labels-idx1-ubyte

biggest_test_emnist : emnist
	mpirun -n 32 ./emnist emnist-digits-train-images-idx3-ubyte emnist-digits-train-labels-idx1-ubyte

clean :
	rm -f $(PROGRAMS) Color.o kmean_color_test.o hw3.o emnist.o IdxIO.o
