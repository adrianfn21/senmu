#ifndef NES_EMULATOR_IMAGE_HPP
#define NES_EMULATOR_IMAGE_HPP

#include <array>
#include <cstdint>

namespace NES {

/**
 * @brief Color struct
 *
 * This struct represents a color in the NES palette. It is composed of 3 bytes, one for each
 * color component (red, green and blue).
 */
struct Color {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
};

/**
 * @brief Palette type
 *
 * This type represents a palette in the NES. It is just an 8-bit unsigned integer.
 */
using Palette = std::uint8_t;

/**
 * @brief Image class
 * @tparam T Type of the image (palette or color)
 * @tparam width Width of the image
 * @tparam height Height of the image
 *
 * This class represents an image in the NES. It is just a wrapper around a std::array.
 */
template <typename T, std::size_t width, std::size_t height>
class Image {
  public:
    static_assert(std::is_same_v<T, Color> || std::is_same_v<T, Palette>, "Template parameter T must be Color or Palette");
    static_assert(width > 0, "Width must be greater than 0");
    static_assert(height > 0, "Height must be greater than 0");

    Image() = default;
    ~Image() = default;

    constexpr T* operator[](std::size_t row) noexcept { return &_data[row * width]; }

    constexpr const T* operator[](std::size_t row) const noexcept { return &_data[row * width]; }

    [[nodiscard]] constexpr T at(std::size_t row, std::size_t col) const noexcept { return _data[row * width + col]; }

    constexpr void set(std::size_t row, std::size_t col, T color) noexcept { _data[row * width + col] = color; }

    [[nodiscard]] constexpr std::array<T, width * height> data() const noexcept { return data; }

  private:
    std::array<T, width * height> _data;
};

}  // namespace NES

#endif  //NES_EMULATOR_IMAGE_HPP
