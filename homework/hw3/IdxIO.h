/**
* @file IdxIO.h - header file for loading data from IDX file
* @author Junwen Zheng
* @date Feb 15, 2026
*/

#pragma once
#include <array>
#include <string>
#include <vector>
#include <cstdint>
#include <istream>

/**
 * Constants describing the fixed dimensions of EMNIST digit images.
 * Each image is a 28x28 grayscale bitmap, stored row-major.
 */
constexpr int kRows = 28;                 // number of rows per image
constexpr int kCols = 28;                 // number of columns per image
constexpr int kDim  = kRows * kCols;      // total pixels per image

/**
 * Type alias for a single image.
 * Each element is one grayscale pixel in the range [0,255].
 */
using Image = std::array<std::uint8_t, kDim>;

/**
 * Container for images read from an image file.
 *
 * Fields:
 *  - count: number of images in the file
 *  - rows: image height
 *  - cols: image width
 *  - images: storage of all images
 */
struct IdxImages {
    int count = 0;            // number of images in the dataset
    int rows = 0;             // rows per image
    int cols = 0;             // columns per image
    std::vector<Image> images; // image data, one Image per entry
};

/**
 * Read a 32-bit unsigned integer in big-endian order.
 *
 * file headers store all integer values in big-endian format,
 *
 * @param in input stream positioned at a 4-byte big-endian integer
 * @return decoded 32-bit unsigned integer value
 */
std::uint32_t read_be_u32(std::istream &in);

/**
 * Read an image file.
 *
 * The file must already be decompressed.
 * Image data is read sequentially into memory and stored as Image values.
 *
 * @param path path to the idx3-ubyte image file
 * @return IdxImages structure containing metadata and all loaded images
 */
IdxImages read_idx3_images(const std::string &path);

/**
 * Read a label file.
 *
 * The file must already be decompressed.
 * Each label is a single byte in the range [0,9].
 *
 * @param path path to the idx1-ubyte label file
 * @return vector of labels, one per image
 */
std::vector<std::uint8_t> read_idx1_labels(const std::string &path);
