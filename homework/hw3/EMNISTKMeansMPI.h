

#pragma once

#include "KMeansMPI.h"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * Concrete MPI-parallel k-means specialization for EMNIST digit.
 *
 * Template parameter:
 *  - k: number of clusters
 */
template <int k>
class EMNISTKMeansMPI : public KMeansMPI<k, 784> {
public:
    /**
     * Element type representing a single digit image.
     */
    using Element = std::array<u_char, 784>;

    /**
     * Run k-means clustering on a set of EMNIST images.
     *
     * This is a convenience wrapper that forwards to the base-class fit()
     * method using the appropriate Element type and dimension.
     *
     * @param images pointer to an array of n digit images
     * @param n number of images in the array
     */
    void fit(Element* images, int n) {
        KMeansMPI<k, 784>::fit(reinterpret_cast<std::array<u_char, 784>*>(images), n);
    }

private:
    using KMeansMPI<k, 784>::fit;

protected:
    /**
     * Compute the distance between two digit images.
     *
     * @param a one digit image
     * @param b another digit image
     * @return Euclidean distance between a and b
     */
    double distance(const Element& a, const Element& b) const override {
        double sum = 0.0;

        for (int i = 0; i < 784; ++i) {
            const double diff = static_cast<double>(a[i]) - static_cast<double>(b[i]);
            sum += diff * diff;
        }

        return std::sqrt(sum);
    }
};