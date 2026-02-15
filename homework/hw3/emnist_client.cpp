//
// Created by Junwen Zheng on 2/15/26.
//

#include "IdxIO.h"
#include <iostream>

int main() {
    auto imgs = read_idx3_images("/Users/zjw/Desktop/CPSC 5600/Remote Repository/cpsc5600/homework/hw3/emnist-digits-train-images-idx3-ubyte");
    auto labels = read_idx1_labels("/Users/zjw/Desktop/CPSC 5600/Remote Repository/cpsc5600/homework/hw3/emnist-digits-train-labels-idx1-ubyte");

    std::cout << "images: " << imgs.count << " (" << imgs.rows << "x" << imgs.cols << ")\n";
    std::cout << "labels: " << labels.size() << "\n";
    std::cout << "first label: " << (int)labels[0] << "\n";
}