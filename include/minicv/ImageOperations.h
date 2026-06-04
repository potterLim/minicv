#pragma once

#include <cstddef>
#include <cstdint>

#include "minicv/Image.h"

namespace minicv
{
	[[nodiscard]] Image CreateAbsoluteDifferenceImage(const Image& left, const Image& right);
	[[nodiscard]] std::size_t CountNonZeroPixels(const Image& image);
	[[nodiscard]] std::uint8_t GetMaximumPixelDifference(const Image& left, const Image& right);
}
