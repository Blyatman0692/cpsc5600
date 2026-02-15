//
// Created by Junwen Zheng on 2/15/26.
//

#include "IdxIO.h"
#include "EMNISTKMeansMPI.h"

const int K = 10;

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: ./emnist_client <images> <labels>\n";
        return 1;
    }

    MPI_Init(nullptr, nullptr);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    EMNISTKMeansMPI<K> emnist;

    if (rank == 0) {
        auto imgs = read_idx3_images(argv[1]);
        auto labels = read_idx1_labels(argv[2]);
        auto images = std::move(imgs.images);

        emnist.fit(images.data(), images.size());
        std::cout << labels[0] << std::endl;
    } else {
        emnist.fitWork(rank);
        MPI_Finalize();
        return 0;
    }

    MPI_Finalize();
}