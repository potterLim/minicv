#include <cassert>
#include <cstddef>
#include <cstdint>

#include "ImageBorder.h"
#include "minicv/Image.h"

namespace minicv::detail
{
	namespace
	{
		bool IsInsideImage(const Image& image, const std::int64_t x, const std::int64_t y)
		{
			return x >= 0 && x < image.GetWidth() && y >= 0 && y < image.GetHeight();
		}

		int ClampImageCoordinate(const std::int64_t coordinate, const int imageLength)
		{
			assert(imageLength > 0 && "image length must be positive.");

			if (coordinate < 0)
			{
				return 0;
			}

			const int maximumCoordinate = imageLength - 1;
			if (coordinate > maximumCoordinate)
			{
				return maximumCoordinate;
			}

			return static_cast<int>(coordinate);
		}

		std::uint8_t GetPixelValue(const Image& image, const int x, const int y, const std::size_t channelIndex)
		{
			const std::size_t pixelByteIndex = CalculatePixelByteIndex(image, x, y, channelIndex);
			return image.GetPixelData()[pixelByteIndex];
		}
	}

	std::size_t CalculatePixelByteIndex(const Image& image, const int x, const int y, const std::size_t channelIndex)
	{
		const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(image.GetBytesPerRow());
		const std::size_t columnOffset = static_cast<std::size_t>(x) * static_cast<std::size_t>(image.GetChannelCount());

		return rowOffset + columnOffset + channelIndex;
	}

	std::uint8_t GetBorderedPixelValue(
		const Image& image,
		const std::int64_t x,
		const std::int64_t y,
		const std::size_t channelIndex,
		const ImageBorderParameters borderParameters)
	{
		assert(!image.IsEmpty() && "border sampling requires a nonempty image.");
		assert(channelIndex < static_cast<std::size_t>(image.GetChannelCount()));
		if (IsInsideImage(image, x, y))
		{
			return GetPixelValue(image, static_cast<int>(x), static_cast<int>(y), channelIndex);
		}

		switch (borderParameters.BorderType)
		{
		case EBorderType::CONSTANT:
			return borderParameters.ConstantBorderValue;

		case EBorderType::REPLICATE:
		{
			const int clampedX = ClampImageCoordinate(x, image.GetWidth());
			const int clampedY = ClampImageCoordinate(y, image.GetHeight());
			return GetPixelValue(image, clampedX, clampedY, channelIndex);
		}

		default:
			assert(false && "unsupported border type.");
			return 0;
		}
	}
}
