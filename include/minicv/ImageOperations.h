#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "minicv/ERgbChannel.h"
#include "minicv/GrayscaleAdaptiveThresholdParameters.h"
#include "minicv/GrayscaleCumulativeDistribution.h"
#include "minicv/GrayscaleHistogram.h"
#include "minicv/GrayscaleThresholdParameters.h"
#include "minicv/GrayscaleValueRange.h"
#include "minicv/Image.h"

namespace minicv
{
	[[nodiscard]] Image CreateAbsoluteDifferenceImage(const Image& left, const Image& right);
	[[nodiscard]] std::size_t CountNonZeroPixels(const Image& image);
	[[nodiscard]] std::uint8_t GetMaximumPixelDifference(const Image& left, const Image& right);

	[[nodiscard]] Image ConvertRgbToGrayscale(const Image& rgbImage);
	[[nodiscard]] Image ConvertGrayscaleToRgb(const Image& grayscaleImage);
	[[nodiscard]] Image CreateRedBlueChannelSwappedImage(const Image& rgbImage);

	[[nodiscard]] Image ExtractRgbChannel(const Image& rgbImage, const ERgbChannel rgbChannel);
	void SplitRgbChannels(
		const Image& rgbImage,
		Image* outRedChannelImage,
		Image* outGreenChannelImage,
		Image* outBlueChannelImage);
	[[nodiscard]] Image MergeRgbChannels(
		const Image& redChannelImage,
		const Image& greenChannelImage,
		const Image& blueChannelImage);

	[[nodiscard]] Image CreateInvertedImage(const Image& image);
	[[nodiscard]] Image AdjustImageBrightness(const Image& image, const int brightnessOffset);
	[[nodiscard]] Image AdjustImageContrast(const Image& image, const float contrastScale);

	[[nodiscard]] GrayscaleHistogram CalculateGrayscaleHistogram(const Image& grayscaleImage);
	[[nodiscard]] GrayscaleCumulativeDistribution CalculateGrayscaleCumulativeDistribution(const GrayscaleHistogram& histogram);
	[[nodiscard]] std::optional<GrayscaleValueRange> TryGetGrayscaleValueRange(const Image& grayscaleImage);
	[[nodiscard]] Image CreateMinMaxNormalizedGrayscaleImage(const Image& grayscaleImage);

	[[nodiscard]] Image CreateContrastStretchedGrayscaleImage(const Image& grayscaleImage, const GrayscaleValueRange valueRange);
	[[nodiscard]] Image CreateHistogramEqualizedGrayscaleImage(const Image& grayscaleImage);
	[[nodiscard]] Image CreateThresholdedGrayscaleImage(const Image& grayscaleImage, const GrayscaleThresholdParameters thresholdParameters);

	[[nodiscard]] std::optional<std::uint8_t> TryCalculateOtsuThreshold(const Image& grayscaleImage);
	[[nodiscard]] Image CreateAdaptiveMeanThresholdedGrayscaleImage(const Image& grayscaleImage, const GrayscaleAdaptiveThresholdParameters thresholdParameters);
}
