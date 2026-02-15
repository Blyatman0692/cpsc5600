#pragma once

#include "KMeansMPI.h"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

template <int k>
class MNISTKMeansMPI : public KMeansMPI<k, 784> {
public:
    using Element = std::array<u_char, 784>;

    void fit(Element* images, int n) {
        // Element already matches the base KMeansMPI Element layout, so cast is safe
        KMeansMPI<k, 784>::fit(reinterpret_cast<std::array<u_char, 784>*>(images), n);
    }


private:
    using KMeansMPI<k, 784>::fit;

protected:

    double distance(const Element& a, const Element& b) const override {
        double sum = 0.0;
        for (int i = 0; i < 784; ++i) {
            const double diff = static_cast<double>(a[i]) - static_cast<double>(b[i]);
            sum += diff * diff;
        }
        return std::sqrt(sum);
    }
};