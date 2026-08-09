#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "minicv/GrayscaleIntegralImage.h"

namespace minicv
{
	namespace
	{
		std::size_t CalculateValuesPerRow(const int width)
		{
			assert(width >= 0 && "width must not be negative.");

			const std::size_t widthSize = static_cast<std::size_t>(width);
			assert(widthSize < std::numeric_limits<std::size_t>::max() && "integral image row size overflow.");

			return widthSize + 1;
		}

		std::size_t CalculateIntegralValueCount(const int width, const int height, const std::size_t valuesPerRow)
		{
			assert(width >= 0 && "width must not be negative.");
			assert(height >= 0 && "height must not be negative.");
			assert(valuesPerRow > 0 && "values per row must be positive.");

			if (width == 0 || height == 0)
			{
				return 0;
			}

			const std::size_t heightSize = static_cast<std::size_t>(height);
			assert(heightSize < std::numeric_limits<std::size_t>::max() && "integral image row count overflow.");

			const std::size_t integralRowCount = heightSize + 1;
			assert(integralRowCount <= std::numeric_limits<std::size_t>::max() / valuesPerRow && "integral image value count overflow.");

			return integralRowCount * valuesPerRow;
		}

		[[maybe_unused]] bool ContainsRegion(const int width, const int height, const Rect region)
		{
			if (region.X < 0 || region.Y < 0 || region.Width < 0 || region.Height < 0)
			{
				return false;
			}

			const std::size_t imageWidth = static_cast<std::size_t>(width);
			const std::size_t imageHeight = static_cast<std::size_t>(height);
			const std::size_t regionX = static_cast<std::size_t>(region.X);
			const std::size_t regionY = static_cast<std::size_t>(region.Y);
			const std::size_t regionWidth = static_cast<std::size_t>(region.Width);
			const std::size_t regionHeight = static_cast<std::size_t>(region.Height);

			const bool fitsHorizontally = regionX <= imageWidth && regionWidth <= imageWidth - regionX;
			const bool fitsVertically = regionY <= imageHeight && regionHeight <= imageHeight - regionY;

			return fitsHorizontally && fitsVertically;
		}
	}

	GrayscaleIntegralImage::GrayscaleIntegralImage(const Image& grayscaleImage)
		: mWidth(grayscaleImage.GetWidth())
		, mHeight(grayscaleImage.GetHeight())
		, mValuesPerRow(CalculateValuesPerRow(mWidth))
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");

		const std::size_t integralValueCount = CalculateIntegralValueCount(mWidth, mHeight, mValuesPerRow);
		assert(integralValueCount <= mIntegralValues.max_size() && "integral image value count exceeds maximum vector size.");

		if (integralValueCount == 0)
		{
			return;
		}

		constexpr std::uint64_t MAX_PIXEL_VALUE = std::numeric_limits<std::uint8_t>::max();
		const std::uint64_t pixelCount = static_cast<std::uint64_t>(grayscaleImage.GetPixelCount());
		const std::uint64_t maximumPixelCount = std::numeric_limits<std::uint64_t>::max() / MAX_PIXEL_VALUE;
		assert(pixelCount <= maximumPixelCount && "integral image pixel sum overflow.");

		(void)pixelCount;
		(void)maximumPixelCount;

		mIntegralValues.resize(integralValueCount);

		const std::uint8_t* const sourcePixelData = grayscaleImage.GetPixelData();
		const std::size_t sourceValuesPerRow = static_cast<std::size_t>(mWidth);

		for (int y = 0; y < mHeight; ++y)
		{
			const std::size_t sourceRowOffset = static_cast<std::size_t>(y) * sourceValuesPerRow;
			const std::size_t previousIntegralRowOffset = static_cast<std::size_t>(y) * mValuesPerRow;
			const std::size_t currentIntegralRowOffset = (static_cast<std::size_t>(y) + 1) * mValuesPerRow;
			std::uint64_t currentRowSum = 0;

			for (int x = 0; x < mWidth; ++x)
			{
				const std::size_t sourceIndex = sourceRowOffset + static_cast<std::size_t>(x);
				const std::size_t integralColumnIndex = static_cast<std::size_t>(x) + 1;
				const std::size_t previousIntegralIndex = previousIntegralRowOffset + integralColumnIndex;
				const std::size_t currentIntegralIndex = currentIntegralRowOffset + integralColumnIndex;

				currentRowSum += sourcePixelData[sourceIndex];
				mIntegralValues[currentIntegralIndex] = currentRowSum + mIntegralValues[previousIntegralIndex];
			}
		}
	}

	bool GrayscaleIntegralImage::IsEmpty() const
	{
		return mIntegralValues.empty();
	}

	int GrayscaleIntegralImage::GetWidth() const
	{
		return mWidth;
	}

	int GrayscaleIntegralImage::GetHeight() const
	{
		return mHeight;
	}

	Size GrayscaleIntegralImage::GetSize() const
	{
		return Size{ mWidth, mHeight };
	}

	std::uint64_t GrayscaleIntegralImage::GetRegionSum(const Rect region) const
	{
		assert(ContainsRegion(mWidth, mHeight, region) && "region must be inside the image.");

		if (region.Width == 0 || region.Height == 0)
		{
			return 0;
		}

		const std::size_t left = static_cast<std::size_t>(region.X);
		const std::size_t top = static_cast<std::size_t>(region.Y);
		const std::size_t right = left + static_cast<std::size_t>(region.Width);
		const std::size_t bottom = top + static_cast<std::size_t>(region.Height);

		const std::size_t topLeftIndex = top * mValuesPerRow + left;
		const std::size_t topRightIndex = top * mValuesPerRow + right;
		const std::size_t bottomLeftIndex = bottom * mValuesPerRow + left;
		const std::size_t bottomRightIndex = bottom * mValuesPerRow + right;

		const std::uint64_t rightStripSum = mIntegralValues[bottomRightIndex] - mIntegralValues[topRightIndex];
		const std::uint64_t leftStripSum = mIntegralValues[bottomLeftIndex] - mIntegralValues[topLeftIndex];

		assert(rightStripSum >= leftStripSum && "integral image region sum underflow.");
		return rightStripSum - leftStripSum;
	}
}
