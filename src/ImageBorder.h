#pragma once

#include <cstddef>
#include <cstdint>

#include "minicv/ImageBorderParameters.h"

namespace minicv
{
	class Image;
}

namespace minicv::detail
{
	[[nodiscard]] std::size_t CalculatePixelByteIndex(const Image& image, const int x, const int y, const std::size_t channelIndex);
	[[nodiscard]] std::uint8_t GetBorderedPixelValue(
		const Image& image,
		const std::int64_t x,
		const std::int64_t y,
		const std::size_t channelIndex,
		const ImageBorderParameters borderParameters);
}
