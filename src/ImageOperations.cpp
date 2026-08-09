#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

#include "minicv/GrayscaleIntegralImage.h"
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

		std::uint8_t NormalizeGrayscalePixel(
			const std::uint8_t pixelValue,
			const std::uint8_t minimumValue,
			const std::uint8_t maximumValue)
		{
			const int valueRange = static_cast<int>(maximumValue) - static_cast<int>(minimumValue);
			assert(valueRange > 0 && "grayscale value range must be positive.");

			if (pixelValue <= minimumValue)
			{
				return std::numeric_limits<std::uint8_t>::min();
			}

			if (pixelValue >= maximumValue)
			{
				return std::numeric_limits<std::uint8_t>::max();
			}

			const int shiftedPixelValue = static_cast<int>(pixelValue) - static_cast<int>(minimumValue);
			const int normalizedMaximum = static_cast<int>(std::numeric_limits<std::uint8_t>::max());
			const int scaledPixelValue = shiftedPixelValue * normalizedMaximum;
			const int roundingOffset = valueRange / 2;

			return static_cast<std::uint8_t>((scaledPixelValue + roundingOffset) / valueRange);
		}

		std::size_t GetFirstPopulatedHistogramBinIndex(const GrayscaleHistogram& histogram)
		{
			for (std::size_t binIndex = 0; binIndex < histogram.BinCounts.size(); ++binIndex)
			{
				if (histogram.BinCounts[binIndex] > 0)
				{
					return binIndex;
				}
			}

			assert(false && "histogram must contain at least one pixel.");
			return 0;
		}

		std::array<std::uint8_t, GRAYSCALE_HISTOGRAM_BIN_COUNT> CreateHistogramEqualizationLookupTable(
			const GrayscaleHistogram& histogram,
			const GrayscaleCumulativeDistribution& cumulativeDistribution)
		{
			std::array<std::uint8_t, GRAYSCALE_HISTOGRAM_BIN_COUNT> lookupTable{};
			const std::size_t firstPopulatedBinIndex = GetFirstPopulatedHistogramBinIndex(histogram);
			const double minimumCumulativeValue = cumulativeDistribution.Values[firstPopulatedBinIndex];
			const double remainingCumulativeRange = 1.0 - minimumCumulativeValue;

			assert(remainingCumulativeRange > 0.0 && "histogram must contain more than one distinct pixel value.");

			const double maximumByteValue = static_cast<double>(std::numeric_limits<std::uint8_t>::max());

			for (std::size_t binIndex = 0; binIndex < lookupTable.size(); ++binIndex)
			{
				const double shiftedCumulativeValue = cumulativeDistribution.Values[binIndex] - minimumCumulativeValue;
				const double equalizedPixelValue = shiftedCumulativeValue / remainingCumulativeRange * maximumByteValue;
				lookupTable[binIndex] = ClampAndRoundToByte(equalizedPixelValue);
			}

			return lookupTable;
		}

		bool IsBinaryThresholdType(const EThresholdType thresholdType)
		{
			return thresholdType == EThresholdType::BINARY || thresholdType == EThresholdType::BINARY_INVERTED;
		}

		std::uint8_t ApplyBinaryThreshold(const bool isAboveThreshold, const EThresholdType thresholdType, const std::uint8_t maximumValue)
		{
			switch (thresholdType)
			{
			case EThresholdType::BINARY:
				return isAboveThreshold ? maximumValue : 0;

			case EThresholdType::BINARY_INVERTED:
				return isAboveThreshold ? 0 : maximumValue;

			default:
				assert(false && "threshold type must be BINARY or BINARY_INVERTED.");
				return 0;
			}
		}

		std::uint8_t ApplyGrayscaleThreshold(const std::uint8_t pixelValue, const GrayscaleThresholdParameters thresholdParameters)
		{
			const bool isAboveThreshold = pixelValue > thresholdParameters.ThresholdValue;

			switch (thresholdParameters.ThresholdType)
			{
			case EThresholdType::BINARY:
			case EThresholdType::BINARY_INVERTED:
				return ApplyBinaryThreshold(isAboveThreshold, thresholdParameters.ThresholdType, thresholdParameters.MaximumValue);

			case EThresholdType::TRUNCATE:
				return isAboveThreshold ? thresholdParameters.ThresholdValue : pixelValue;

			case EThresholdType::TO_ZERO:
				return isAboveThreshold ? pixelValue : 0;

			case EThresholdType::TO_ZERO_INVERTED:
				return isAboveThreshold ? 0 : pixelValue;

			default:
				assert(false && "unsupported threshold type.");
				return 0;
			}
		}

		Rect CreateClippedNeighborhoodRegion(const int x, const int y, const int blockRadius, const Size imageSize)
		{
			const std::int64_t leftCandidate = static_cast<std::int64_t>(x) - blockRadius;
			const std::int64_t topCandidate = static_cast<std::int64_t>(y) - blockRadius;
			const std::int64_t rightCandidate = static_cast<std::int64_t>(x) + blockRadius + 1;
			const std::int64_t bottomCandidate = static_cast<std::int64_t>(y) + blockRadius + 1;

			const int left = leftCandidate > 0 ? static_cast<int>(leftCandidate) : 0;
			const int top = topCandidate > 0 ? static_cast<int>(topCandidate) : 0;
			const int right = rightCandidate < imageSize.Width ? static_cast<int>(rightCandidate) : imageSize.Width;
			const int bottom = bottomCandidate < imageSize.Height ? static_cast<int>(bottomCandidate) : imageSize.Height;

			return Rect{ left, top, right - left, bottom - top };
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

	GrayscaleHistogram CalculateGrayscaleHistogram(const Image& grayscaleImage)
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");

		GrayscaleHistogram histogram{};
		const std::uint8_t* const pixelData = grayscaleImage.GetPixelData();
		const std::size_t pixelCount = grayscaleImage.GetPixelCount();

		for (std::size_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
		{
			const std::size_t binIndex = static_cast<std::size_t>(pixelData[pixelIndex]);
			++histogram.BinCounts[binIndex];
		}

		return histogram;
	}

	GrayscaleCumulativeDistribution CalculateGrayscaleCumulativeDistribution(const GrayscaleHistogram& histogram)
	{
		GrayscaleCumulativeDistribution cumulativeDistribution{};
		long double totalPixelCount = 0.0L;

		for (const std::size_t binCount : histogram.BinCounts)
		{
			totalPixelCount += static_cast<long double>(binCount);
		}

		if (totalPixelCount == 0.0L)
		{
			return cumulativeDistribution;
		}

		long double cumulativePixelCount = 0.0L;

		for (std::size_t binIndex = 0; binIndex < histogram.BinCounts.size(); ++binIndex)
		{
			cumulativePixelCount += static_cast<long double>(histogram.BinCounts[binIndex]);
			cumulativeDistribution.Values[binIndex] = static_cast<double>(cumulativePixelCount / totalPixelCount);
		}

		return cumulativeDistribution;
	}

	std::optional<GrayscaleValueRange> TryGetGrayscaleValueRange(const Image& grayscaleImage)
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");

		if (grayscaleImage.IsEmpty())
		{
			return std::nullopt;
		}

		const std::uint8_t* const pixelData = grayscaleImage.GetPixelData();
		const std::size_t pixelCount = grayscaleImage.GetPixelCount();
		std::uint8_t minimumValue = pixelData[0];
		std::uint8_t maximumValue = pixelData[0];

		for (std::size_t pixelIndex = 1; pixelIndex < pixelCount; ++pixelIndex)
		{
			const std::uint8_t pixelValue = pixelData[pixelIndex];

			if (pixelValue < minimumValue)
			{
				minimumValue = pixelValue;
			}

			if (pixelValue > maximumValue)
			{
				maximumValue = pixelValue;
			}
		}

		return GrayscaleValueRange{ minimumValue, maximumValue };
	}

	Image CreateMinMaxNormalizedGrayscaleImage(const Image& grayscaleImage)
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");

		Image normalizedImage(grayscaleImage.GetSize(), EImageType::UINT8_GRAYSCALE);
		const std::optional<GrayscaleValueRange> valueRange = TryGetGrayscaleValueRange(grayscaleImage);

		if (!valueRange.has_value() || valueRange->Minimum == valueRange->Maximum)
		{
			return normalizedImage;
		}

		const std::uint8_t* const sourcePixelData = grayscaleImage.GetPixelData();
		std::uint8_t* const normalizedPixelData = normalizedImage.GetPixelData();
		const std::size_t pixelCount = grayscaleImage.GetPixelCount();

		for (std::size_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
		{
			normalizedPixelData[pixelIndex] = NormalizeGrayscalePixel(sourcePixelData[pixelIndex], valueRange->Minimum, valueRange->Maximum);
		}

		return normalizedImage;
	}

	Image CreateContrastStretchedGrayscaleImage(const Image& grayscaleImage, const GrayscaleValueRange valueRange)
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");
		assert(valueRange.Minimum < valueRange.Maximum && "valueRange must have a positive range.");

		Image stretchedImage(grayscaleImage.GetSize(), EImageType::UINT8_GRAYSCALE);
		const std::uint8_t* const sourcePixelData = grayscaleImage.GetPixelData();
		std::uint8_t* const stretchedPixelData = stretchedImage.GetPixelData();
		const std::size_t pixelCount = grayscaleImage.GetPixelCount();

		for (std::size_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
		{
			stretchedPixelData[pixelIndex] = NormalizeGrayscalePixel(sourcePixelData[pixelIndex], valueRange.Minimum, valueRange.Maximum);
		}

		return stretchedImage;
	}

	Image CreateHistogramEqualizedGrayscaleImage(const Image& grayscaleImage)
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");

		if (grayscaleImage.IsEmpty())
		{
			return Image(grayscaleImage.GetSize(), EImageType::UINT8_GRAYSCALE);
		}

		const GrayscaleHistogram histogram = CalculateGrayscaleHistogram(grayscaleImage);
		const std::size_t firstPopulatedBinIndex = GetFirstPopulatedHistogramBinIndex(histogram);

		if (histogram.BinCounts[firstPopulatedBinIndex] == grayscaleImage.GetPixelCount())
		{
			return grayscaleImage.Clone();
		}

		const GrayscaleCumulativeDistribution cumulativeDistribution = CalculateGrayscaleCumulativeDistribution(histogram);
		const std::array<std::uint8_t, GRAYSCALE_HISTOGRAM_BIN_COUNT> lookupTable = CreateHistogramEqualizationLookupTable(histogram, cumulativeDistribution);
		Image equalizedImage(grayscaleImage.GetSize(), EImageType::UINT8_GRAYSCALE);
		const std::uint8_t* const sourcePixelData = grayscaleImage.GetPixelData();
		std::uint8_t* const equalizedPixelData = equalizedImage.GetPixelData();
		const std::size_t pixelCount = grayscaleImage.GetPixelCount();

		for (std::size_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
		{
			const std::size_t lookupIndex = static_cast<std::size_t>(sourcePixelData[pixelIndex]);
			equalizedPixelData[pixelIndex] = lookupTable[lookupIndex];
		}

		return equalizedImage;
	}

	Image CreateThresholdedGrayscaleImage(const Image& grayscaleImage, const GrayscaleThresholdParameters thresholdParameters)
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");

		Image thresholdedImage(grayscaleImage.GetSize(), EImageType::UINT8_GRAYSCALE);
		const std::uint8_t* const sourcePixelData = grayscaleImage.GetPixelData();
		std::uint8_t* const thresholdedPixelData = thresholdedImage.GetPixelData();
		const std::size_t pixelCount = grayscaleImage.GetPixelCount();

		for (std::size_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
		{
			thresholdedPixelData[pixelIndex] = ApplyGrayscaleThreshold(sourcePixelData[pixelIndex], thresholdParameters);
		}

		return thresholdedImage;
	}

	std::optional<std::uint8_t> TryCalculateOtsuThreshold(const Image& grayscaleImage)
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");

		if (grayscaleImage.IsEmpty())
		{
			return std::nullopt;
		}

		const GrayscaleHistogram histogram = CalculateGrayscaleHistogram(grayscaleImage);
		const long double totalPixelCount = static_cast<long double>(grayscaleImage.GetPixelCount());
		long double totalWeightedPixelValue = 0.0L;

		for (std::size_t binIndex = 0; binIndex < histogram.BinCounts.size(); ++binIndex)
		{
			totalWeightedPixelValue += static_cast<long double>(binIndex) * static_cast<long double>(histogram.BinCounts[binIndex]);
		}

		long double backgroundPixelCount = 0.0L;
		long double backgroundWeightedPixelValue = 0.0L;
		long double maximumBetweenClassVariance = -1.0L;
		std::uint8_t otsuThreshold = 0;

		for (std::size_t binIndex = 0; binIndex < histogram.BinCounts.size(); ++binIndex)
		{
			const long double binCount = static_cast<long double>(histogram.BinCounts[binIndex]);
			backgroundPixelCount += binCount;
			backgroundWeightedPixelValue += static_cast<long double>(binIndex) * binCount;

			if (backgroundPixelCount == 0.0L)
			{
				continue;
			}

			const long double foregroundPixelCount = totalPixelCount - backgroundPixelCount;
			if (foregroundPixelCount == 0.0L)
			{
				break;
			}

			const long double backgroundMean = backgroundWeightedPixelValue / backgroundPixelCount;
			const long double foregroundWeightedPixelValue = totalWeightedPixelValue - backgroundWeightedPixelValue;
			const long double foregroundMean = foregroundWeightedPixelValue / foregroundPixelCount;
			const long double meanDifference = backgroundMean - foregroundMean;
			const long double betweenClassVariance = backgroundPixelCount * foregroundPixelCount * meanDifference * meanDifference;

			if (betweenClassVariance > maximumBetweenClassVariance)
			{
				maximumBetweenClassVariance = betweenClassVariance;
				otsuThreshold = static_cast<std::uint8_t>(binIndex);
			}
		}

		return otsuThreshold;
	}

	Image CreateAdaptiveMeanThresholdedGrayscaleImage(const Image& grayscaleImage, const GrayscaleAdaptiveThresholdParameters thresholdParameters)
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");

		const bool isBinaryThresholdType = IsBinaryThresholdType(thresholdParameters.ThresholdType);
		assert(isBinaryThresholdType && "threshold type must be BINARY or BINARY_INVERTED.");
		assert(thresholdParameters.BlockSize >= 3 && "block size must be at least 3.");
		assert(thresholdParameters.BlockSize % 2 == 1 && "block size must be odd.");
		assert(std::isfinite(thresholdParameters.MeanOffset) && "mean offset must be finite.");

		(void)isBinaryThresholdType;

		Image thresholdedImage(grayscaleImage.GetSize(), EImageType::UINT8_GRAYSCALE);
		if (grayscaleImage.IsEmpty())
		{
			return thresholdedImage;
		}

		const GrayscaleIntegralImage integralImage(grayscaleImage);
		const int blockRadius = thresholdParameters.BlockSize / 2;
		const Size imageSize = grayscaleImage.GetSize();

		for (int y = 0; y < imageSize.Height; ++y)
		{
			for (int x = 0; x < imageSize.Width; ++x)
			{
				const Rect neighborhoodRegion = CreateClippedNeighborhoodRegion(x, y, blockRadius, imageSize);
				const std::uint64_t neighborhoodPixelSum = integralImage.GetRegionSum(neighborhoodRegion);
				const std::size_t neighborhoodPixelCount = static_cast<std::size_t>(neighborhoodRegion.Width) * static_cast<std::size_t>(neighborhoodRegion.Height);
				const long double neighborhoodMean = static_cast<long double>(neighborhoodPixelSum) / static_cast<long double>(neighborhoodPixelCount);
				const long double localThreshold = neighborhoodMean - static_cast<long double>(thresholdParameters.MeanOffset);
				const std::uint8_t pixelValue = grayscaleImage.GetGrayscalePixel(x, y);
				const bool isAboveThreshold = static_cast<long double>(pixelValue) > localThreshold;

				thresholdedImage.GetGrayscalePixel(x, y) = ApplyBinaryThreshold(isAboveThreshold, thresholdParameters.ThresholdType, thresholdParameters.MaximumValue);
			}
		}

		return thresholdedImage;
	}
}
