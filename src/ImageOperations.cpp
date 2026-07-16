#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

#include "minicv/ImageOperations.h"

namespace minicv
{
	namespace
	{
		constexpr int GRAYSCALE_RED_WEIGHT = 299;
		constexpr int GRAYSCALE_GREEN_WEIGHT = 587;
		constexpr int GRAYSCALE_BLUE_WEIGHT = 114;
		constexpr int GRAYSCALE_WEIGHT_SCALE = 1000;
		constexpr int GRAYSCALE_ROUNDING_OFFSET = GRAYSCALE_WEIGHT_SCALE / 2;

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

		std::uint8_t ConvertRgbPixelToGrayscale(
			const std::uint8_t red,
			const std::uint8_t green,
			const std::uint8_t blue)
		{
			const int weightedSum =
				red * GRAYSCALE_RED_WEIGHT +
				green * GRAYSCALE_GREEN_WEIGHT +
				blue * GRAYSCALE_BLUE_WEIGHT +
				GRAYSCALE_ROUNDING_OFFSET;

			return static_cast<std::uint8_t>(weightedSum / GRAYSCALE_WEIGHT_SCALE);
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

			return static_cast<std::uint8_t>(std::lround(pixelValue));
		}

		Image CreateLinearTransformedImage(const Image& image, const double scale, const double offset)
		{
			Image transformedImage(image.GetSize(), image.GetImageType());
			const std::uint8_t* const sourcePixelData = image.GetPixelData();
			std::uint8_t* const transformedPixelData = transformedImage.GetPixelData();
			const std::size_t byteCount = image.GetByteCount();

			for (std::size_t index = 0; index < byteCount; ++index)
			{
				const double transformedPixelValue = static_cast<double>(sourcePixelData[index]) * scale + offset;
				transformedPixelData[index] = ClampAndRoundToByte(transformedPixelValue);
			}

			return transformedImage;
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

	Image ConvertRgbToGrayscale(const Image& rgbImage)
	{
		assert(rgbImage.GetImageType() == EImageType::UINT8_RGB && "image type must be UINT8_RGB.");

		Image grayscaleImage(rgbImage.GetSize(), EImageType::UINT8_GRAYSCALE);

		for (int y = 0; y < rgbImage.GetHeight(); ++y)
		{
			for (int x = 0; x < rgbImage.GetWidth(); ++x)
			{
				const std::uint8_t red = rgbImage.GetRgbPixel(x, y, ERgbChannel::RED);
				const std::uint8_t green = rgbImage.GetRgbPixel(x, y, ERgbChannel::GREEN);
				const std::uint8_t blue = rgbImage.GetRgbPixel(x, y, ERgbChannel::BLUE);

				grayscaleImage.GetGrayscalePixel(x, y) = ConvertRgbPixelToGrayscale(red, green, blue);
			}
		}

		return grayscaleImage;
	}

	Image ConvertGrayscaleToRgb(const Image& grayscaleImage)
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");

		Image rgbImage(grayscaleImage.GetSize(), EImageType::UINT8_RGB);

		for (int y = 0; y < grayscaleImage.GetHeight(); ++y)
		{
			for (int x = 0; x < grayscaleImage.GetWidth(); ++x)
			{
				const std::uint8_t pixelValue = grayscaleImage.GetGrayscalePixel(x, y);

				rgbImage.GetRgbPixel(x, y, ERgbChannel::RED) = pixelValue;
				rgbImage.GetRgbPixel(x, y, ERgbChannel::GREEN) = pixelValue;
				rgbImage.GetRgbPixel(x, y, ERgbChannel::BLUE) = pixelValue;
			}
		}

		return rgbImage;
	}

	Image CreateRedBlueChannelSwappedImage(const Image& rgbImage)
	{
		assert(rgbImage.GetImageType() == EImageType::UINT8_RGB && "image type must be UINT8_RGB.");

		Image swappedImage(rgbImage.GetSize(), EImageType::UINT8_RGB);

		for (int y = 0; y < rgbImage.GetHeight(); ++y)
		{
			for (int x = 0; x < rgbImage.GetWidth(); ++x)
			{
				swappedImage.GetRgbPixel(x, y, ERgbChannel::RED) = rgbImage.GetRgbPixel(x, y, ERgbChannel::BLUE);
				swappedImage.GetRgbPixel(x, y, ERgbChannel::GREEN) = rgbImage.GetRgbPixel(x, y, ERgbChannel::GREEN);
				swappedImage.GetRgbPixel(x, y, ERgbChannel::BLUE) = rgbImage.GetRgbPixel(x, y, ERgbChannel::RED);
			}
		}

		return swappedImage;
	}

	Image ExtractRgbChannel(const Image& rgbImage, const ERgbChannel rgbChannel)
	{
		assert(rgbImage.GetImageType() == EImageType::UINT8_RGB && "image type must be UINT8_RGB.");

		Image channelImage(rgbImage.GetSize(), EImageType::UINT8_GRAYSCALE);

		for (int y = 0; y < rgbImage.GetHeight(); ++y)
		{
			for (int x = 0; x < rgbImage.GetWidth(); ++x)
			{
				channelImage.GetGrayscalePixel(x, y) = rgbImage.GetRgbPixel(x, y, rgbChannel);
			}
		}

		return channelImage;
	}

	void SplitRgbChannels(
		const Image& rgbImage,
		Image* outRedChannelImage,
		Image* outGreenChannelImage,
		Image* outBlueChannelImage)
	{
		assert(outRedChannelImage != nullptr && "outRedChannelImage must not be null.");
		assert(outGreenChannelImage != nullptr && "outGreenChannelImage must not be null.");
		assert(outBlueChannelImage != nullptr && "outBlueChannelImage must not be null.");
		assert(outRedChannelImage != outGreenChannelImage && "output channel images must be distinct.");
		assert(outRedChannelImage != outBlueChannelImage && "output channel images must be distinct.");
		assert(outGreenChannelImage != outBlueChannelImage && "output channel images must be distinct.");
		assert(rgbImage.GetImageType() == EImageType::UINT8_RGB && "image type must be UINT8_RGB.");

		Image redChannelImage = ExtractRgbChannel(rgbImage, ERgbChannel::RED);
		Image greenChannelImage = ExtractRgbChannel(rgbImage, ERgbChannel::GREEN);
		Image blueChannelImage = ExtractRgbChannel(rgbImage, ERgbChannel::BLUE);

		*outRedChannelImage = std::move(redChannelImage);
		*outGreenChannelImage = std::move(greenChannelImage);
		*outBlueChannelImage = std::move(blueChannelImage);
	}

	Image MergeRgbChannels(
		const Image& redChannelImage,
		const Image& greenChannelImage,
		const Image& blueChannelImage)
	{
		assert(redChannelImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "red channel must be UINT8_GRAYSCALE.");
		assert(greenChannelImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "green channel must be UINT8_GRAYSCALE.");
		assert(blueChannelImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "blue channel must be UINT8_GRAYSCALE.");
		assert(redChannelImage.HasSameSize(greenChannelImage) && "channel images must have same size.");
		assert(redChannelImage.HasSameSize(blueChannelImage) && "channel images must have same size.");

		Image rgbImage(redChannelImage.GetSize(), EImageType::UINT8_RGB);

		for (int y = 0; y < rgbImage.GetHeight(); ++y)
		{
			for (int x = 0; x < rgbImage.GetWidth(); ++x)
			{
				rgbImage.GetRgbPixel(x, y, ERgbChannel::RED) = redChannelImage.GetGrayscalePixel(x, y);
				rgbImage.GetRgbPixel(x, y, ERgbChannel::GREEN) = greenChannelImage.GetGrayscalePixel(x, y);
				rgbImage.GetRgbPixel(x, y, ERgbChannel::BLUE) = blueChannelImage.GetGrayscalePixel(x, y);
			}
		}

		return rgbImage;
	}

	Image CreateInvertedImage(const Image& image)
	{
		const double maximumByteValue = static_cast<double>(std::numeric_limits<std::uint8_t>::max());
		return CreateLinearTransformedImage(image, -1.0, maximumByteValue);
	}

	Image AdjustImageBrightness(const Image& image, const int brightnessOffset)
	{
		return CreateLinearTransformedImage(image, 1.0, static_cast<double>(brightnessOffset));
	}

	Image AdjustImageContrast(const Image& image, const float contrastScale)
	{
		assert(std::isfinite(contrastScale) && "contrastScale must be finite.");
		assert(contrastScale >= 0.0f && "contrastScale must not be negative.");

		return CreateLinearTransformedImage(image, static_cast<double>(contrastScale), 0.0);
	}
}
