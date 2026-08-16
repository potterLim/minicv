#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "minicv/ConvolutionKernel.h"
#include "minicv/ImageFiltering.h"

namespace minicv
{
	namespace
	{
		bool IsSupportedBorderType(const EBorderType borderType)
		{
			return borderType == EBorderType::CONSTANT || borderType == EBorderType::REPLICATE;
		}

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

		std::size_t CalculatePixelByteIndex(const Image& image, const int x, const int y, const std::size_t channelIndex)
		{
			const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(image.GetBytesPerRow());
			const std::size_t columnOffset = static_cast<std::size_t>(x) * static_cast<std::size_t>(image.GetChannelCount());

			return rowOffset + columnOffset + channelIndex;
		}

		std::uint8_t GetPixelValue(const Image& image, const int x, const int y, const std::size_t channelIndex)
		{
			const std::size_t pixelByteIndex = CalculatePixelByteIndex(image, x, y, channelIndex);
			return image.GetPixelData()[pixelByteIndex];
		}

		std::uint8_t GetBorderedPixelValue(
			const Image& image,
			const std::int64_t x,
			const std::int64_t y,
			const std::size_t channelIndex,
			const ImageBorderParameters borderParameters)
		{
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

		std::uint8_t ClampAndRoundToByte(const double pixelValue)
		{
			const std::uint8_t minimumByteValue = std::numeric_limits<std::uint8_t>::min();
			const std::uint8_t maximumByteValue = std::numeric_limits<std::uint8_t>::max();

			if (pixelValue <= static_cast<double>(minimumByteValue))
			{
				return minimumByteValue;
			}

			if (pixelValue >= static_cast<double>(maximumByteValue))
			{
				return maximumByteValue;
			}

			const bool isPixelValueFinite = std::isfinite(pixelValue);
			assert(isPixelValueFinite && "convolved pixel value must be finite.");

			if (!isPixelValueFinite)
			{
				return minimumByteValue;
			}

			return static_cast<std::uint8_t>(std::lround(pixelValue));
		}

		double ConvolvePixelChannel(
			const Image& image,
			const ConvolutionKernel& kernel,
			const int x,
			const int y,
			const std::size_t channelIndex,
			const ImageBorderParameters borderParameters)
		{
			const int kernelCenterX = kernel.GetWidth() / 2;
			const int kernelCenterY = kernel.GetHeight() / 2;
			double convolvedPixelValue = 0.0;

			for (int kernelY = 0; kernelY < kernel.GetHeight(); ++kernelY)
			{
				for (int kernelX = 0; kernelX < kernel.GetWidth(); ++kernelX)
				{
					const std::int64_t sourceX = static_cast<std::int64_t>(x) + kernelCenterX - kernelX;
					const std::int64_t sourceY = static_cast<std::int64_t>(y) + kernelCenterY - kernelY;
					const std::uint8_t sourcePixelValue = GetBorderedPixelValue(image, sourceX, sourceY, channelIndex, borderParameters);
					const double coefficient = kernel.GetCoefficient(kernelX, kernelY);

					convolvedPixelValue += static_cast<double>(sourcePixelValue) * coefficient;
				}
			}

			return convolvedPixelValue;
		}
	}

	Image CreateConvolvedImage(const Image& image, const ConvolutionKernel& kernel, const ImageBorderParameters borderParameters)
	{
		const bool isBorderTypeSupported = IsSupportedBorderType(borderParameters.BorderType);
		assert(isBorderTypeSupported && "border type must be CONSTANT or REPLICATE.");

		(void)isBorderTypeSupported;

		Image convolvedImage(image.GetSize(), image.GetImageType());
		if (image.IsEmpty())
		{
			return convolvedImage;
		}

		std::uint8_t* const convolvedPixelData = convolvedImage.GetPixelData();
		const std::size_t channelCount = static_cast<std::size_t>(image.GetChannelCount());

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				for (std::size_t channelIndex = 0; channelIndex < channelCount; ++channelIndex)
				{
					const double convolvedPixelValue = ConvolvePixelChannel(image, kernel, x, y, channelIndex, borderParameters);
					const std::size_t pixelByteIndex = CalculatePixelByteIndex(convolvedImage, x, y, channelIndex);

					convolvedPixelData[pixelByteIndex] = ClampAndRoundToByte(convolvedPixelValue);
				}
			}
		}

		return convolvedImage;
	}
}
