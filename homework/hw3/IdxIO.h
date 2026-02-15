#pragma once

#include <array>
#include <string>
#include <vector>
#include <cstdint>
#include <istream>

constexpr int kRows = 28;
constexpr int kCols = 28;
constexpr int kDim = kRows * kCols;
using Image = std::array<std::uint8_t, kDim>;

struct IdxImages {
    int count = 0;
    int rows = 0;
    int cols = 0;
    std::vector<Image> images;
};

std::uint32_t read_be_u32(std::istream &in);

IdxImages read_idx3_images(const std::string &path);

std::vector<std::uint8_t> read_idx1_labels(const std::string &path);

