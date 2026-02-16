/**
* @file emnist.cpp - driver code for k-means clustering algorithm on EMNIST digits dataset
* @author Junwen Zheng
* @date Feb 15, 2026
*/

#include "IdxIO.h"
#include "EMNISTKMeansMPI.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <utility>

/**
 * Driver for EMNIST digits clustering using MPI-parallel k-means.
 *
 * This program:
 *  1) Loads an EMNIST IDX3 image file and IDX1 label file on ROOT,
 *  2) Runs k-means (k=10) in parallel via EMNISTKMeansMPI (built on KMeansMPI),
 *  3) Prints a short report of the converged clustering result.
 *
 * Usage:
 *   ./emnist <images> <labels>
 */

/** K should be fixed to 10 since we have digits 0..9. */
constexpr int K = 10;

int main(int argc, char** argv) {
    // Validate arguments.
    if (argc < 3) {
        std::cerr << "Usage: ./emnist <images> <labels>\n";
        return 1;
    }

    MPI_Init(nullptr, nullptr);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    EMNISTKMeansMPI<K> emnist;

    if (rank == 0) {
        // ROOT loads the full dataset from disk.
        auto imgs = read_idx3_images(argv[1]);
        auto labels = read_idx1_labels(argv[2]);

        // move large image vector out of the container.
        auto images = std::move(imgs.images);

        // ROOT starts the k-means run.
        emnist.fit(images.data(), static_cast<int>(images.size()));

        // ----- converged results -----
        const auto &clusters = emnist.getClusters();

        int correct = 0;
        int total = 0;

        // For each cluster, compute the majority digit label and report cluster size.
        for (int c = 0; c < K; c++) {
            std::array<int, 10> freq{};  // digit frequency table for labels 0..9

            for (int idx : clusters[c].elements) {
                if (idx >= 0 && idx < static_cast<int>(labels.size())) {
                    const int d = static_cast<int>(labels[idx]);
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