#include <cassert>
#include <cstddef>
#include <cstdint>

#include "minicv/ImageOperations.h"

namespace minicv
{
	namespace
	{
		std::uint8_t GetAbsoluteDifference(const std::uint8_t left, const std::uint8_t right)
		{
			if (left >= right)
			{
				return static_cast<std::uint8_t>(left - right);
			}

			return static_cast<std::uint8_t>(right - left);
		}

		bool HasNonZeroChannel(const Image& image, const std::size_t pixelIndex)
		{
			const std::uint8_t* const pixelData = image.GetPixelData();
			const std::size_t channelCount = static_cast<std::size_t>(image.GetChannelCount());
			const std::size_t byteIndex = pixelIndex * channelCount;

			for (std::size_t channelIndex = 0; channelIndex < channelCount; ++channelIndex)
			{
				if (pixelData[byteIndex + channelIndex] != 0)
				{
					return true;
				}
			}

			return false;
		}
	}

	Image CreateAbsoluteDifferenceImage(const Image& left, const Image& right)
	{
		assert(left.HasSameShape(right) && "images must have same shape.");

		Image differenceImage(left.GetSize(), left.GetImageType());
		const std::uint8_t* const leftPixelData = left.GetPixelData();
		const std::uint8_t* const rightPixelData = right.GetPixelData();
		std::uint8_t* const differencePixelData = differenceImage.GetPixelData();
		const std::size_t byteCount = left.GetByteCount();

		for (std::size_t index = 0; index < byteCount; ++index)
		{
			differencePixelData[index] = GetAbsoluteDifference(leftPixelData[index], rightPixelData[index]);
		}

		return differenceImage;
	}

	std::size_t CountNonZeroPixels(const Image& image)
	{
		std::size_t nonZeroPixelCount = 0;
		const std::size_t pixelCount = image.GetPixelCount();

		for (std::size_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
		{
			if (HasNonZeroChannel(image, pixelIndex))
			{
				++nonZeroPixelCount;
			}
		}

		return nonZeroPixelCount;
	}

	std::uint8_t GetMaximumPixelDifference(const Image& left, const Image& right)
	{
		assert(left.HasSameShape(right) && "images must have same shape.");

		const std::uint8_t* const leftPixelData = left.GetPixelData();
		const std::uint8_t* const rightPixelData = right.GetPixelData();
		const std::size_t byteCount = left.GetByteCount();
		std::uint8_t maximumDifference = 0;

		for (std::size_t index = 0; index < byteCount; ++index)
		{
			const std::uint8_t difference = GetAbsoluteDifference(leftPixelData[index], rightPixelData[index]);
			if (difference > maximumDifference)
			{
				maximumDifference = difference;
			}
		}

		return maximumDifference;
	}
}
