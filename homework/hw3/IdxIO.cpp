#include "IdxIO.h"

#include <fstream>
#include <stdexcept>
#include <sstream>

std::uint32_t read_be_u32(std::istream& in) {
    std::uint8_t b[4];
    in.read(reinterpret_cast<char*>(b), 4);
    if (!in) throw std::runtime_error("Malformed input file.");
    return (std::uint32_t(b[0]) << 24) |
           (std::uint32_t(b[1]) << 16) |
           (std::uint32_t(b[2]) <<  8) |
            std::uint32_t(b[3]);
}

// EMNIST often needs transpose/flip to match typical MNIST orientation.
// This helper does: transpose (r,c)->(c,r) then flip horizontally.
// If you don’t care about visualization, clustering still “works” without it,
// but fixing it makes results more interpretable.
Image fix_emnist_image_orientation(const Image& src) {
    Image dst{};
    // src is row-major [r*cols + c] with rows=28 cols=28
    // transpose + horizontal flip:
    // dst[r,c] = src[c, 27-r]  (one common EMNIST correction)
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            int dst_idx = r * kCols + c;
            int src_r = c;
            int src_c = (kCols - 1) - r;
            int src_idx = src_r * kCols + src_c;
            dst[dst_idx] = src[src_idx];
        }
    }
    return dst;
}

IdxImages read_idx3_images(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Could not open file.");
    }
    const std::uint32_t magic = read_be_u32(in);
    if (magic != 2051) {
        std::ostringstream oss;
        oss << "Bad magic for idx3 images. Got " << magic;
        throw std::runtime_error(oss.str());
    }

    const std::uint32_t count = read_be_u32(in);
    const std::uint32_t rows  = read_be_u32(in);
    const std::uint32_t cols  = read_be_u32(in);

    if (rows != kRows || cols != kCols) {
        std::ostringstream oss;
        oss << "Malformed image shape. Expected 28x28, got " << rows << "x" << cols;
        throw std::runtime_error(oss.str());
    }

    IdxImages out;
    out.count = static_cast<int>(count);
    out.rows  = static_cast<int>(rows);
    out.cols  = static_cast<int>(cols);
    out.images.resize(out.count);

    for (int i = 0; i < out.count; i++) {
        Image img{};
        in.read(reinterpret_cast<char*>(img.data()), kDim);
        if (!in) {
            throw std::runtime_error("Malformed image bytes");
        }
        out.images[i] = img;

    }

    return out;
}

std::vector<std::uint8_t> read_idx1_labels(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Could not open file.");
    }

    const std::uint32_t magic = read_be_u32(in);
    if (magic != 2049) {
        std::ostringstream oss;
        oss << "Bad magic for idx1 labels. Got " << magic;
        throw std::runtime_error(oss.str());
    }

    const std::uint32_t count = read_be_u32(in);

    std::vector<std::uint8_t> labels(count);
    in.read(reinterpret_cast<char*>(labels.data()), count);
    if (!in) {
        throw std::runtime_error("Malformed image labels");
    }

    return labels;
}