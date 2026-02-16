/**
* @file IdxIO.cpp - Implementation of methods for loading data from IDX file
* @author Junwen Zheng
* @date Feb 15, 2026
*/

#include "IdxIO.h"
#include <fstream>
#include <stdexcept>
#include <sstream>

/**
 * Read a 32-bit unsigned integer from a binary file in big-endian order.
 *
 * @param in input stream positioned at a 4-byte big-endian integer
 * @return decoded 32-bit unsigned integer
 * @throws std::runtime_error if the stream does not contain 4 readable bytes
 */
std::uint32_t read_be_u32(std::istream& in) {
    std::uint8_t b[4];

    // Read exactly four bytes from the stream
    in.read(reinterpret_cast<char*>(b), 4);
    if (!in) {
        throw std::runtime_error("Malformed input file.");
    }

    // Reconstruct a 32-bit value from big-endian byte order
    return (std::uint32_t(b[0]) << 24) |
           (std::uint32_t(b[1]) << 16) |
           (std::uint32_t(b[2]) <<  8) |
            std::uint32_t(b[3]);
}

/**
 * Read an IDX3 image file containing.
 *
 * The file must already be decompressed. The function validates
 * the IDX magic number and image dimensions, then reads all image data
 * sequentially into memory.
 *
 * @param path filesystem path to the idx3-ubyte image file
 * @return IdxImages structure containing metadata and all loaded images
 * @throws std::runtime_error if the file cannot be opened or is malformed
 */
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

    // Read dataset metadata
    const std::uint32_t count = read_be_u32(in);
    const std::uint32_t rows  = read_be_u32(in);
    const std::uint32_t cols  = read_be_u32(in);

    // Expected image dimensions
    if (rows != kRows || cols != kCols) {
        std::ostringstream oss;
        oss << "Malformed image shape. Expected 28x28, got "
            << rows << "x" << cols;
        throw std::runtime_error(oss.str());
    }

    IdxImages out;
    out.count = static_cast<int>(count);
    out.rows  = static_cast<int>(rows);
    out.cols  = static_cast<int>(cols);
    out.images.resize(out.count);

    // Read image data
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

/**
 * Read an IDX1 label file.
 *
 * The file must already be decompressed. Each label is stored
 * as a single byte with a value in the range [0,9].
 *
 * @param path filesystem path to the idx1-ubyte label file
 * @return vector of labels, one per image
 * @throws std::runtime_error if the file cannot be opened or is malformed
 */
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

    // Number of labels in the file
    const std::uint32_t count = read_be_u32(in);

    std::vector<std::uint8_t> labels(count);

    // Read one byte per label
    in.read(reinterpret_cast<char*>(labels.data()), count);
    if (!in) {
        throw std::runtime_error("Malformed image labels");
    }

    return labels;
}