#pragma once

#include <cstddef>
#include <cstdint>

#include "minicv/ERgbChannel.h"
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
}
