//
// Created by Junwen Zheng on 2/15/26.
//

#include "IdxIO.h"
#include "EMNISTKMeansMPI.h"

// K should be fixed to 10 since we have digits 0..9
constexpr int K = 10;

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

        // report converged results
        const auto &clusters = emnist.getClusters();

        int correct = 0;
        int total = 0;

        for (int c = 0; c < K; c++) {
            std::array<int, 10> freq{};
            for (int idx : clusters[c].elements) {
                if (idx >= 0 && idx < static_cast<int>(labels.size())) {
                    int d = static_cast<int>(labels[idx]);
                    if (0 <= d && d < 10) {
                        freq[d]++;
                    }
                }
            }

            const int maj = static_cast<int>(std::distance(freq.begin(), std::max_element(freq.begin(), freq.end())));
            std::cout << "Cluster " << c
                      << ": size = " << clusters[c].elements.size()
                      << ", majority digit = " << maj
                      << std::endl;

            // contribute to accuracy
            for (int idx : clusters[c].elements) {
                if (idx >= 0 && idx < static_cast<int>(labels.size())) {
                    total++;
                    if (static_cast<int>(labels[idx]) == maj) {
                        correct++;
                    }
                }
            }
        }

        if (total > 0) {
            std::cout << "Accuracy: "
                      << (100.0 * static_cast<double>(correct) / static_cast<double>(total))
                      << "%" << std::endl;
        }


    } else {
        emnist.fitWork(rank);
        MPI_Finalize();
        return 0;
    }

    MPI_Finalize();
    return 0;
}