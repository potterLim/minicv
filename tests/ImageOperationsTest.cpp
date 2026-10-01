#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>

#include "ImageOperationsTest.h"
#include "minicv/Image.h"
#include "minicv/ImageOperations.h"

namespace
{
	void SetRgbPixel(
		minicv::Image& image,
		const int x,
		const int y,
		const std::uint8_t red,
		const std::uint8_t green,
		const std::uint8_t blue)
	{
		image.GetRgbPixel(x, y, minicv::ERgbChannel::RED) = red;
		image.GetRgbPixel(x, y, minicv::ERgbChannel::GREEN) = green;
		image.GetRgbPixel(x, y, minicv::ERgbChannel::BLUE) = blue;
	}

	void AssertRgbPixelEquals(
		const minicv::Image& image,
		const int x,
		const int y,
		const std::uint8_t red,
		const std::uint8_t green,
		const std::uint8_t blue)
	{
		const bool isRedEqual = image.GetRgbPixel(x, y, minicv::ERgbChannel::RED) == red;
		const bool isGreenEqual = image.GetRgbPixel(x, y, minicv::ERgbChannel::GREEN) == green;
		const bool isBlueEqual = image.GetRgbPixel(x, y, minicv::ERgbChannel::BLUE) == blue;

		assert(isRedEqual);
		assert(isGreenEqual);
		assert(isBlueEqual);

		(void)isRedEqual;
		(void)isGreenEqual;
		(void)isBlueEqual;
	}

	void AssertHistogramBinCount(
		const minicv::GrayscaleHistogram& histogram,
		const std::uint8_t pixelValue,
		const std::size_t expectedBinCount)
	{
		const std::size_t binIndex = static_cast<std::size_t>(pixelValue);
		const bool isBinCountEqual = histogram.BinCounts[binIndex] == expectedBinCount;

		assert(isBinCountEqual);

		(void)isBinCountEqual;
	}

	void AssertCumulativeDistributionValue(
		const minicv::GrayscaleCumulativeDistribution& cumulativeDistribution,
		const std::uint8_t pixelValue,
		const double expectedValue)
	{
		const std::size_t binIndex = static_cast<std::size_t>(pixelValue);
		const bool isValueEqual = cumulativeDistribution.Values[binIndex] == expectedValue;

		assert(isValueEqual);

		(void)isValueEqual;
	}

	void AssertCumulativeDistributionIsValid(const minicv::GrayscaleCumulativeDistribution& cumulativeDistribution)
	{
		double previousValue = 0.0;

		for (const double cumulativeValue : cumulativeDistribution.Values)
		{
			const bool isValueInRange = cumulativeValue >= 0.0 && cumulativeValue <= 1.0;
			const bool isNonDecreasing = cumulativeValue >= previousValue;

			assert(isValueInRange);
			assert(isNonDecreasing);

			(void)isValueInRange;
			(void)isNonDecreasing;

			previousValue = cumulativeValue;
		}
	}

	template <std::size_t PIXEL_COUNT>
	void AssertGrayscaleRowEquals(
		const minicv::Image& image,
		const std::array<std::uint8_t, PIXEL_COUNT>& expectedPixelValues)
	{
		const bool isWidthEqual = image.GetWidth() == static_cast<int>(PIXEL_COUNT);
		const bool isHeightEqual = image.GetHeight() == 1;

		assert(isWidthEqual);
		assert(isHeightEqual);

		(void)isWidthEqual;
		(void)isHeightEqual;

		for (std::size_t pixelIndex = 0; pixelIndex < PIXEL_COUNT; ++pixelIndex)
		{
			const int x = static_cast<int>(pixelIndex);
			const bool isPixelEqual = image.GetGrayscalePixel(x, 0) == expectedPixelValues[pixelIndex];

			assert(isPixelEqual);

			(void)isPixelEqual;
		}
	}

	void TestCreateAbsoluteDifferenceImage()
	{
		minicv::Image leftImage(3, 1);
		minicv::Image rightImage(3, 1);

		leftImage.GetGrayscalePixel(0, 0) = 10;
		leftImage.GetGrayscalePixel(1, 0) = 100;
		leftImage.GetGrayscalePixel(2, 0) = 250;

		rightImage.GetGrayscalePixel(0, 0) = 25;
		rightImage.GetGrayscalePixel(1, 0) = 90;
		rightImage.GetGrayscalePixel(2, 0) = 200;

		const minicv::Image differenceImage = minicv::CreateAbsoluteDifferenceImage(leftImage, rightImage);

		assert(differenceImage.GetWidth() == 3);
		assert(differenceImage.GetHeight() == 1);
		assert(differenceImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(differenceImage.GetGrayscalePixel(0, 0) == 15);
		assert(differenceImage.GetGrayscalePixel(1, 0) == 10);
		assert(differenceImage.GetGrayscalePixel(2, 0) == 50);

		assert(leftImage.GetGrayscalePixel(0, 0) == 10);
		assert(rightImage.GetGrayscalePixel(0, 0) == 25);
	}

	void TestCreateAbsoluteDifferenceRgbImage()
	{
		minicv::Image leftImage(1, 1, minicv::EImageType::UINT8_RGB);
		minicv::Image rightImage(1, 1, minicv::EImageType::UINT8_RGB);

		leftImage.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) = 10;
		leftImage.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) = 100;
		leftImage.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) = 250;

		rightImage.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) = 25;
		rightImage.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) = 90;
		rightImage.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) = 200;

		const minicv::Image differenceImage = minicv::CreateAbsoluteDifferenceImage(leftImage, rightImage);

		assert(differenceImage.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(differenceImage.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) == 15);
		assert(differenceImage.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) == 10);
		assert(differenceImage.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) == 50);
	}

	void TestCreateAbsoluteDifferenceImageFromEmptyImages()
	{
		const minicv::Image leftImage;
		const minicv::Image rightImage;
		const minicv::Image differenceImage = minicv::CreateAbsoluteDifferenceImage(leftImage, rightImage);

		assert(differenceImage.IsEmpty());
		assert(differenceImage.GetWidth() == 0);
		assert(differenceImage.GetHeight() == 0);
		assert(differenceImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
	}

	void TestCountNonZeroPixels()
	{
		minicv::Image image(4, 1);

		image.GetGrayscalePixel(0, 0) = 0;
		image.GetGrayscalePixel(1, 0) = 1;
		image.GetGrayscalePixel(2, 0) = 0;
		image.GetGrayscalePixel(3, 0) = 255;

		assert(minicv::CountNonZeroPixels(image) == 2);
	}

	void TestCountNonZeroRgbPixels()
	{
		minicv::Image image(3, 1, minicv::EImageType::UINT8_RGB);

		image.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) = 0;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) = 0;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) = 0;

		image.GetRgbPixel(1, 0, minicv::ERgbChannel::RED) = 10;
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::GREEN) = 20;
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::BLUE) = 30;

		image.GetRgbPixel(2, 0, minicv::ERgbChannel::RED) = 0;
		image.GetRgbPixel(2, 0, minicv::ERgbChannel::GREEN) = 20;
		image.GetRgbPixel(2, 0, minicv::ERgbChannel::BLUE) = 0;

		assert(minicv::CountNonZeroPixels(image) == 2);
	}

	void TestCountNonZeroPixelsFromEmptyImage()
	{
		const minicv::Image image;

		assert(minicv::CountNonZeroPixels(image) == 0);
	}

	void TestGetMaximumPixelDifference()
	{
		minicv::Image leftImage(3, 1);
		minicv::Image rightImage(3, 1);

		leftImage.GetGrayscalePixel(0, 0) = 10;
		leftImage.GetGrayscalePixel(1, 0) = 100;
		leftImage.GetGrayscalePixel(2, 0) = 200;

		rightImage.GetGrayscalePixel(0, 0) = 20;
		rightImage.GetGrayscalePixel(1, 0) = 90;
		rightImage.GetGrayscalePixel(2, 0) = 250;

		assert(minicv::GetMaximumPixelDifference(leftImage, rightImage) == 50);
	}

	void TestGetMaximumPixelDifferenceFromEmptyImages()
	{
		const minicv::Image leftImage;
		const minicv::Image rightImage;

		assert(minicv::GetMaximumPixelDifference(leftImage, rightImage) == 0);
	}

	void TestConvertRgbToGrayscale()
	{
		minicv::Image rgbImage(6, 1, minicv::EImageType::UINT8_RGB);

		SetRgbPixel(rgbImage, 0, 0, 0, 0, 0);
		SetRgbPixel(rgbImage, 1, 0, 255, 0, 0);
		SetRgbPixel(rgbImage, 2, 0, 0, 255, 0);
		SetRgbPixel(rgbImage, 3, 0, 0, 0, 255);
		SetRgbPixel(rgbImage, 4, 0, 255, 255, 255);
		SetRgbPixel(rgbImage, 5, 0, 100, 150, 200);

		const minicv::Image grayscaleImage = minicv::ConvertRgbToGrayscale(rgbImage);

		assert(grayscaleImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(grayscaleImage.GetWidth() == rgbImage.GetWidth());
		assert(grayscaleImage.GetHeight() == rgbImage.GetHeight());
		assert(grayscaleImage.GetGrayscalePixel(0, 0) == 0);
		assert(grayscaleImage.GetGrayscalePixel(1, 0) == 76);
		assert(grayscaleImage.GetGrayscalePixel(2, 0) == 150);
		assert(grayscaleImage.GetGrayscalePixel(3, 0) == 29);
		assert(grayscaleImage.GetGrayscalePixel(4, 0) == 255);
		assert(grayscaleImage.GetGrayscalePixel(5, 0) == 141);

		AssertRgbPixelEquals(rgbImage, 5, 0, 100, 150, 200);
	}

	void TestConvertGrayscaleToRgb()
	{
		minicv::Image grayscaleImage(3, 1);
		grayscaleImage.GetGrayscalePixel(0, 0) = 0;
		grayscaleImage.GetGrayscalePixel(1, 0) = 128;
		grayscaleImage.GetGrayscalePixel(2, 0) = 255;

		const minicv::Image rgbImage = minicv::ConvertGrayscaleToRgb(grayscaleImage);

		assert(rgbImage.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(rgbImage.GetWidth() == grayscaleImage.GetWidth());
		assert(rgbImage.GetHeight() == grayscaleImage.GetHeight());
		AssertRgbPixelEquals(rgbImage, 0, 0, 0, 0, 0);
		AssertRgbPixelEquals(rgbImage, 1, 0, 128, 128, 128);
		AssertRgbPixelEquals(rgbImage, 2, 0, 255, 255, 255);

		assert(grayscaleImage.GetGrayscalePixel(1, 0) == 128);
	}

	void TestCreateRedBlueChannelSwappedImage()
	{
		minicv::Image rgbImage(2, 1, minicv::EImageType::UINT8_RGB);
		SetRgbPixel(rgbImage, 0, 0, 10, 20, 30);
		SetRgbPixel(rgbImage, 1, 0, 40, 50, 60);

		const minicv::Image swappedImage = minicv::CreateRedBlueChannelSwappedImage(rgbImage);

		assert(swappedImage.GetImageType() == minicv::EImageType::UINT8_RGB);
		AssertRgbPixelEquals(swappedImage, 0, 0, 30, 20, 10);
		AssertRgbPixelEquals(swappedImage, 1, 0, 60, 50, 40);
		AssertRgbPixelEquals(rgbImage, 0, 0, 10, 20, 30);
	}

	void TestExtractRgbChannel()
	{
		minicv::Image rgbImage(2, 1, minicv::EImageType::UINT8_RGB);
		SetRgbPixel(rgbImage, 0, 0, 10, 20, 30);
		SetRgbPixel(rgbImage, 1, 0, 40, 50, 60);

		const minicv::Image redChannelImage = minicv::ExtractRgbChannel(rgbImage, minicv::ERgbChannel::RED);
		const minicv::Image greenChannelImage = minicv::ExtractRgbChannel(rgbImage, minicv::ERgbChannel::GREEN);
		const minicv::Image blueChannelImage = minicv::ExtractRgbChannel(rgbImage, minicv::ERgbChannel::BLUE);

		assert(redChannelImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(redChannelImage.GetGrayscalePixel(0, 0) == 10);
		assert(redChannelImage.GetGrayscalePixel(1, 0) == 40);
		assert(greenChannelImage.GetGrayscalePixel(0, 0) == 20);
		assert(greenChannelImage.GetGrayscalePixel(1, 0) == 50);
		assert(blueChannelImage.GetGrayscalePixel(0, 0) == 30);
		assert(blueChannelImage.GetGrayscalePixel(1, 0) == 60);
	}

	void TestSplitRgbChannels()
	{
		minicv::Image rgbImage(2, 1, minicv::EImageType::UINT8_RGB);
		SetRgbPixel(rgbImage, 0, 0, 10, 20, 30);
		SetRgbPixel(rgbImage, 1, 0, 40, 50, 60);

		minicv::Image redChannelImage;
		minicv::Image greenChannelImage;
		minicv::Image blueChannelImage;

		minicv::SplitRgbChannels(rgbImage, &redChannelImage, &greenChannelImage, &blueChannelImage);

		assert(redChannelImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(greenChannelImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(blueChannelImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(redChannelImage.GetGrayscalePixel(0, 0) == 10);
		assert(redChannelImage.GetGrayscalePixel(1, 0) == 40);
		assert(greenChannelImage.GetGrayscalePixel(0, 0) == 20);
		assert(greenChannelImage.GetGrayscalePixel(1, 0) == 50);
		assert(blueChannelImage.GetGrayscalePixel(0, 0) == 30);
		assert(blueChannelImage.GetGrayscalePixel(1, 0) == 60);
	}

	void TestSplitRgbChannelsWithSourceAsOutput()
	{
		minicv::Image rgbImage(2, 1, minicv::EImageType::UINT8_RGB);
		SetRgbPixel(rgbImage, 0, 0, 10, 20, 30);
		SetRgbPixel(rgbImage, 1, 0, 40, 50, 60);

		minicv::Image greenChannelImage;
		minicv::Image blueChannelImage;

		minicv::SplitRgbChannels(rgbImage, &rgbImage, &greenChannelImage, &blueChannelImage);

		assert(rgbImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(rgbImage.GetGrayscalePixel(0, 0) == 10);
		assert(rgbImage.GetGrayscalePixel(1, 0) == 40);
		assert(greenChannelImage.GetGrayscalePixel(0, 0) == 20);
		assert(greenChannelImage.GetGrayscalePixel(1, 0) == 50);
		assert(blueChannelImage.GetGrayscalePixel(0, 0) == 30);
		assert(blueChannelImage.GetGrayscalePixel(1, 0) == 60);
	}

	void TestMergeRgbChannels()
	{
		minicv::Image redChannelImage(2, 2);
		minicv::Image greenChannelImage(2, 2);
		minicv::Image blueChannelImage(2, 2);

		redChannelImage.GetGrayscalePixel(0, 0) = 10;
		redChannelImage.GetGrayscalePixel(1, 0) = 40;
		redChannelImage.GetGrayscalePixel(0, 1) = 70;
		redChannelImage.GetGrayscalePixel(1, 1) = 100;

		greenChannelImage.GetGrayscalePixel(0, 0) = 20;
		greenChannelImage.GetGrayscalePixel(1, 0) = 50;
		greenChannelImage.GetGrayscalePixel(0, 1) = 80;
		greenChannelImage.GetGrayscalePixel(1, 1) = 110;

		blueChannelImage.GetGrayscalePixel(0, 0) = 30;
		blueChannelImage.GetGrayscalePixel(1, 0) = 60;
		blueChannelImage.GetGrayscalePixel(0, 1) = 90;
		blueChannelImage.GetGrayscalePixel(1, 1) = 120;

		const minicv::Image rgbImage = minicv::MergeRgbChannels(redChannelImage, greenChannelImage, blueChannelImage);

		assert(rgbImage.GetImageType() == minicv::EImageType::UINT8_RGB);
		AssertRgbPixelEquals(rgbImage, 0, 0, 10, 20, 30);
		AssertRgbPixelEquals(rgbImage, 1, 0, 40, 50, 60);
		AssertRgbPixelEquals(rgbImage, 0, 1, 70, 80, 90);
		AssertRgbPixelEquals(rgbImage, 1, 1, 100, 110, 120);
	}

	void TestSplitAndMergeRgbChannels()
	{
		minicv::Image rgbImage(2, 1, minicv::EImageType::UINT8_RGB);
		SetRgbPixel(rgbImage, 0, 0, 10, 20, 30);
		SetRgbPixel(rgbImage, 1, 0, 40, 50, 60);

		minicv::Image redChannelImage;
		minicv::Image greenChannelImage;
		minicv::Image blueChannelImage;

		minicv::SplitRgbChannels(rgbImage, &redChannelImage, &greenChannelImage, &blueChannelImage);
		const minicv::Image mergedImage = minicv::MergeRgbChannels(redChannelImage, greenChannelImage, blueChannelImage);

		assert(mergedImage.HasSameContent(rgbImage));
	}

	void TestCreateInvertedImage()
	{
		minicv::Image image(3, 1);
		image.GetGrayscalePixel(0, 0) = 0;
		image.GetGrayscalePixel(1, 0) = 127;
		image.GetGrayscalePixel(2, 0) = 255;

		const minicv::Image invertedImage = minicv::CreateInvertedImage(image);

		assert(invertedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(invertedImage.GetGrayscalePixel(0, 0) == 255);
		assert(invertedImage.GetGrayscalePixel(1, 0) == 128);
		assert(invertedImage.GetGrayscalePixel(2, 0) == 0);
		assert(image.GetGrayscalePixel(1, 0) == 127);

		(void)invertedImage;
	}

	void TestAdjustImageBrightness()
	{
		minicv::Image image(4, 1);
		image.GetGrayscalePixel(0, 0) = 0;
		image.GetGrayscalePixel(1, 0) = 10;
		image.GetGrayscalePixel(2, 0) = 240;
		image.GetGrayscalePixel(3, 0) = 255;

		const minicv::Image brighterImage = minicv::AdjustImageBrightness(image, 20);
		const minicv::Image darkerImage = minicv::AdjustImageBrightness(image, -20);

		assert(brighterImage.GetGrayscalePixel(0, 0) == 20);
		assert(brighterImage.GetGrayscalePixel(1, 0) == 30);
		assert(brighterImage.GetGrayscalePixel(2, 0) == 255);
		assert(brighterImage.GetGrayscalePixel(3, 0) == 255);

		assert(darkerImage.GetGrayscalePixel(0, 0) == 0);
		assert(darkerImage.GetGrayscalePixel(1, 0) == 0);
		assert(darkerImage.GetGrayscalePixel(2, 0) == 220);
		assert(darkerImage.GetGrayscalePixel(3, 0) == 235);

		assert(image.GetGrayscalePixel(2, 0) == 240);

		(void)brighterImage;
		(void)darkerImage;
	}

	void TestAdjustImageBrightnessWithExtremeOffsets()
	{
		minicv::Image image(2, 1);
		image.GetGrayscalePixel(0, 0) = 0;
		image.GetGrayscalePixel(1, 0) = 255;

		const minicv::Image maximumBrightnessImage = minicv::AdjustImageBrightness(image, std::numeric_limits<int>::max());
		const minicv::Image minimumBrightnessImage = minicv::AdjustImageBrightness(image, std::numeric_limits<int>::min());

		assert(maximumBrightnessImage.GetGrayscalePixel(0, 0) == 255);
		assert(maximumBrightnessImage.GetGrayscalePixel(1, 0) == 255);
		assert(minimumBrightnessImage.GetGrayscalePixel(0, 0) == 0);
		assert(minimumBrightnessImage.GetGrayscalePixel(1, 0) == 0);
		assert(image.GetGrayscalePixel(0, 0) == 0);
		assert(image.GetGrayscalePixel(1, 0) == 255);

		(void)maximumBrightnessImage;
		(void)minimumBrightnessImage;
	}

	void TestAdjustImageContrast()
	{
		minicv::Image image(5, 1);
		image.GetGrayscalePixel(0, 0) = 0;
		image.GetGrayscalePixel(1, 0) = 1;
		image.GetGrayscalePixel(2, 0) = 127;
		image.GetGrayscalePixel(3, 0) = 200;
		image.GetGrayscalePixel(4, 0) = 255;

		const minicv::Image identityImage = minicv::AdjustImageContrast(image, 1.0f);
		const minicv::Image increasedContrastImage = minicv::AdjustImageContrast(image, 1.5f);
		const minicv::Image zeroContrastImage = minicv::AdjustImageContrast(image, 0.0f);

		assert(identityImage.HasSameContent(image));
		assert(increasedContrastImage.GetGrayscalePixel(0, 0) == 0);
		assert(increasedContrastImage.GetGrayscalePixel(1, 0) == 2);
		assert(increasedContrastImage.GetGrayscalePixel(2, 0) == 191);
		assert(increasedContrastImage.GetGrayscalePixel(3, 0) == 255);
		assert(increasedContrastImage.GetGrayscalePixel(4, 0) == 255);

		for (int x = 0; x < zeroContrastImage.GetWidth(); ++x)
		{
			assert(zeroContrastImage.GetGrayscalePixel(x, 0) == 0);
		}

		(void)identityImage;
		(void)increasedContrastImage;
		(void)zeroContrastImage;
	}

	void TestPixelValueOperationsWithRgbImage()
	{
		minicv::Image image(1, 1, minicv::EImageType::UINT8_RGB);
		SetRgbPixel(image, 0, 0, 10, 100, 250);

		const minicv::Image invertedImage = minicv::CreateInvertedImage(image);
		const minicv::Image brighterImage = minicv::AdjustImageBrightness(image, 20);
		const minicv::Image increasedContrastImage = minicv::AdjustImageContrast(image, 1.5f);

		AssertRgbPixelEquals(invertedImage, 0, 0, 245, 155, 5);
		AssertRgbPixelEquals(brighterImage, 0, 0, 30, 120, 255);
		AssertRgbPixelEquals(increasedContrastImage, 0, 0, 15, 150, 255);
		AssertRgbPixelEquals(image, 0, 0, 10, 100, 250);
	}

	void TestPixelValueOperationsFromEmptyImages()
	{
		const minicv::Image emptyGrayscaleImage;
		const minicv::Image emptyRgbImage(0, 0, minicv::EImageType::UINT8_RGB);

		const minicv::Image invertedImage = minicv::CreateInvertedImage(emptyGrayscaleImage);
		const minicv::Image brighterImage = minicv::AdjustImageBrightness(emptyRgbImage, 20);
		const minicv::Image contrastImage = minicv::AdjustImageContrast(emptyRgbImage, 1.5f);

		assert(invertedImage.IsEmpty());
		assert(invertedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(brighterImage.IsEmpty());
		assert(brighterImage.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(contrastImage.IsEmpty());
		assert(contrastImage.GetImageType() == minicv::EImageType::UINT8_RGB);

		(void)invertedImage;
		(void)brighterImage;
		(void)contrastImage;
	}

	void TestCalculateGrayscaleHistogram()
	{
		minicv::Image image(7, 1);
		image.GetGrayscalePixel(0, 0) = 0;
		image.GetGrayscalePixel(1, 0) = 0;
		image.GetGrayscalePixel(2, 0) = 1;
		image.GetGrayscalePixel(3, 0) = 127;
		image.GetGrayscalePixel(4, 0) = 255;
		image.GetGrayscalePixel(5, 0) = 255;
		image.GetGrayscalePixel(6, 0) = 255;

		const minicv::GrayscaleHistogram histogram = minicv::CalculateGrayscaleHistogram(image);

		AssertHistogramBinCount(histogram, 0, 2);
		AssertHistogramBinCount(histogram, 1, 1);
		AssertHistogramBinCount(histogram, 2, 0);
		AssertHistogramBinCount(histogram, 127, 1);
		AssertHistogramBinCount(histogram, 255, 3);

		std::size_t totalPixelCount = 0;
		for (const std::size_t binCount : histogram.BinCounts)
		{
			totalPixelCount += binCount;
		}

		assert(totalPixelCount == image.GetPixelCount());

		(void)totalPixelCount;
	}

	void TestCalculateGrayscaleHistogramFromEmptyImage()
	{
		const minicv::Image image;
		const minicv::GrayscaleHistogram histogram = minicv::CalculateGrayscaleHistogram(image);

		for (const std::size_t binCount : histogram.BinCounts)
		{
			const bool isBinEmpty = binCount == 0;
			assert(isBinEmpty);
			(void)isBinEmpty;
		}
	}

	void TestCalculateGrayscaleCumulativeDistribution()
	{
		minicv::GrayscaleHistogram histogram{};
		histogram.BinCounts[0] = 1;
		histogram.BinCounts[1] = 1;
		histogram.BinCounts[255] = 2;

		const minicv::GrayscaleCumulativeDistribution cumulativeDistribution = minicv::CalculateGrayscaleCumulativeDistribution(histogram);

		AssertCumulativeDistributionValue(cumulativeDistribution, 0, 0.25);
		AssertCumulativeDistributionValue(cumulativeDistribution, 1, 0.5);
		AssertCumulativeDistributionValue(cumulativeDistribution, 127, 0.5);
		AssertCumulativeDistributionValue(cumulativeDistribution, 254, 0.5);
		AssertCumulativeDistributionValue(cumulativeDistribution, 255, 1.0);
		AssertCumulativeDistributionIsValid(cumulativeDistribution);
	}

	void TestCalculateGrayscaleCumulativeDistributionFromEmptyHistogram()
	{
		const minicv::GrayscaleHistogram histogram{};
		const minicv::GrayscaleCumulativeDistribution cumulativeDistribution = minicv::CalculateGrayscaleCumulativeDistribution(histogram);

		for (const double cumulativeValue : cumulativeDistribution.Values)
		{
			const bool isValueZero = cumulativeValue == 0.0;
			assert(isValueZero);
			(void)isValueZero;
		}

		AssertCumulativeDistributionIsValid(cumulativeDistribution);
	}

	void TestTryGetGrayscaleValueRange()
	{
		minicv::Image image(4, 1);
		image.GetGrayscalePixel(0, 0) = 200;
		image.GetGrayscalePixel(1, 0) = 10;
		image.GetGrayscalePixel(2, 0) = 90;
		image.GetGrayscalePixel(3, 0) = 255;

		const std::optional<minicv::GrayscaleValueRange> valueRange = minicv::TryGetGrayscaleValueRange(image);
		const std::optional<minicv::GrayscaleValueRange> emptyValueRange = minicv::TryGetGrayscaleValueRange(minicv::Image{});

		assert(valueRange.has_value());
		assert(valueRange->Minimum == 10);
		assert(valueRange->Maximum == 255);
		assert(!emptyValueRange.has_value());

		(void)valueRange;
		(void)emptyValueRange;
	}

	void TestCreateMinMaxNormalizedGrayscaleImage()
	{
		minicv::Image image(5, 1);
		image.GetGrayscalePixel(0, 0) = 10;
		image.GetGrayscalePixel(1, 0) = 12;
		image.GetGrayscalePixel(2, 0) = 15;
		image.GetGrayscalePixel(3, 0) = 18;
		image.GetGrayscalePixel(4, 0) = 20;

		const minicv::Image normalizedImage = minicv::CreateMinMaxNormalizedGrayscaleImage(image);

		assert(normalizedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(normalizedImage.GetSize().Width == image.GetSize().Width);
		assert(normalizedImage.GetSize().Height == image.GetSize().Height);
		assert(normalizedImage.GetGrayscalePixel(0, 0) == 0);
		assert(normalizedImage.GetGrayscalePixel(1, 0) == 51);
		assert(normalizedImage.GetGrayscalePixel(2, 0) == 128);
		assert(normalizedImage.GetGrayscalePixel(3, 0) == 204);
		assert(normalizedImage.GetGrayscalePixel(4, 0) == 255);
		assert(image.GetGrayscalePixel(2, 0) == 15);

		(void)normalizedImage;
	}

	void TestCreateMinMaxNormalizedGrayscaleImageFromConstantImage()
	{
		minicv::Image image(3, 1);
		image.Fill(42);

		const std::optional<minicv::GrayscaleValueRange> valueRange = minicv::TryGetGrayscaleValueRange(image);
		const minicv::Image normalizedImage = minicv::CreateMinMaxNormalizedGrayscaleImage(image);

		assert(valueRange.has_value());
		assert(valueRange->Minimum == 42);
		assert(valueRange->Maximum == 42);

		for (std::size_t pixelIndex = 0; pixelIndex < normalizedImage.GetPixelCount(); ++pixelIndex)
		{
			const bool isPixelZero = normalizedImage.GetPixelData()[pixelIndex] == 0;
			assert(isPixelZero);
			(void)isPixelZero;
		}

		(void)valueRange;
	}

	void TestCreateMinMaxNormalizedGrayscaleImageFromEmptyImage()
	{
		const minicv::Image image;
		const minicv::Image normalizedImage = minicv::CreateMinMaxNormalizedGrayscaleImage(image);

		assert(normalizedImage.IsEmpty());
		assert(normalizedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);

		(void)normalizedImage;
	}

	void TestGrayscaleHistogramAndNormalizationFlow()
	{
		minicv::Image sourceImage(2, 2);
		sourceImage.GetGrayscalePixel(0, 0) = 50;
		sourceImage.GetGrayscalePixel(1, 0) = 50;
		sourceImage.GetGrayscalePixel(0, 1) = 100;
		sourceImage.GetGrayscalePixel(1, 1) = 150;

		const minicv::GrayscaleHistogram sourceHistogram = minicv::CalculateGrayscaleHistogram(sourceImage);
		const minicv::GrayscaleCumulativeDistribution sourceCumulativeDistribution = minicv::CalculateGrayscaleCumulativeDistribution(sourceHistogram);
		const std::optional<minicv::GrayscaleValueRange> sourceValueRange = minicv::TryGetGrayscaleValueRange(sourceImage);
		const minicv::Image normalizedImage = minicv::CreateMinMaxNormalizedGrayscaleImage(sourceImage);
		const minicv::GrayscaleHistogram normalizedHistogram = minicv::CalculateGrayscaleHistogram(normalizedImage);
		const std::optional<minicv::GrayscaleValueRange> normalizedValueRange = minicv::TryGetGrayscaleValueRange(normalizedImage);

		AssertHistogramBinCount(sourceHistogram, 50, 2);
		AssertHistogramBinCount(sourceHistogram, 100, 1);
		AssertHistogramBinCount(sourceHistogram, 150, 1);
		AssertCumulativeDistributionValue(sourceCumulativeDistribution, 49, 0.0);
		AssertCumulativeDistributionValue(sourceCumulativeDistribution, 50, 0.5);
		AssertCumulativeDistributionValue(sourceCumulativeDistribution, 100, 0.75);
		AssertCumulativeDistributionValue(sourceCumulativeDistribution, 150, 1.0);
		AssertCumulativeDistributionIsValid(sourceCumulativeDistribution);

		assert(sourceValueRange.has_value());
		assert(sourceValueRange->Minimum == 50);
		assert(sourceValueRange->Maximum == 150);
		assert(normalizedImage.GetGrayscalePixel(0, 0) == 0);
		assert(normalizedImage.GetGrayscalePixel(1, 0) == 0);
		assert(normalizedImage.GetGrayscalePixel(0, 1) == 128);
		assert(normalizedImage.GetGrayscalePixel(1, 1) == 255);
		assert(normalizedValueRange.has_value());
		assert(normalizedValueRange->Minimum == 0);
		assert(normalizedValueRange->Maximum == 255);
		assert(sourceImage.GetGrayscalePixel(0, 1) == 100);

		AssertHistogramBinCount(normalizedHistogram, 0, 2);
		AssertHistogramBinCount(normalizedHistogram, 128, 1);
		AssertHistogramBinCount(normalizedHistogram, 255, 1);

		(void)sourceValueRange;
		(void)normalizedImage;
		(void)normalizedValueRange;
	}

	void TestCreateContrastStretchedGrayscaleImage()
	{
		minicv::Image image(6, 1);
		image.GetGrayscalePixel(0, 0) = 10;
		image.GetGrayscalePixel(1, 0) = 50;
		image.GetGrayscalePixel(2, 0) = 100;
		image.GetGrayscalePixel(3, 0) = 150;
		image.GetGrayscalePixel(4, 0) = 200;
		image.GetGrayscalePixel(5, 0) = 250;

		const minicv::GrayscaleValueRange valueRange{ 50, 200 };
		const minicv::Image stretchedImage = minicv::CreateContrastStretchedGrayscaleImage(image, valueRange);
		const std::array<std::uint8_t, 6> expectedPixelValues{ 0, 0, 85, 170, 255, 255 };

		AssertGrayscaleRowEquals(stretchedImage, expectedPixelValues);
		assert(stretchedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(image.GetGrayscalePixel(2, 0) == 100);

		(void)stretchedImage;
	}

	void TestCreateHistogramEqualizedGrayscaleImage()
	{
		minicv::Image image(4, 1);
		image.GetGrayscalePixel(0, 0) = 50;
		image.GetGrayscalePixel(1, 0) = 50;
		image.GetGrayscalePixel(2, 0) = 100;
		image.GetGrayscalePixel(3, 0) = 150;

		const minicv::Image equalizedImage = minicv::CreateHistogramEqualizedGrayscaleImage(image);
		const std::array<std::uint8_t, 4> expectedPixelValues{ 0, 0, 128, 255 };

		AssertGrayscaleRowEquals(equalizedImage, expectedPixelValues);
		assert(equalizedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(image.GetGrayscalePixel(2, 0) == 100);

		(void)equalizedImage;
	}

	void TestHistogramEqualizationRoundsFromPixelCounts()
	{
		for (int firstCount = 1; firstCount <= 10; ++firstCount)
		{
			for (int middleCount = 1; middleCount <= 10; ++middleCount)
			{
				for (int lastCount = 1; lastCount <= 10; ++lastCount)
				{
					minicv::Image image(firstCount + middleCount + lastCount, 1);
					for (int x = 0; x < image.GetWidth(); ++x)
					{
						image.GetGrayscalePixel(x, 0) = x < firstCount ? 0 : x < firstCount + middleCount ? 1 : 2;
					}
					const minicv::Image equalizedImage = minicv::CreateHistogramEqualizedGrayscaleImage(image);
					const int remainingCount = middleCount + lastCount;
					const int expectedMiddleValue = (255 * middleCount + remainingCount / 2) / remainingCount;
					for (int x = 0; x < image.GetWidth(); ++x)
					{
						const int expectedValue = x < firstCount ? 0 : x < firstCount + middleCount ? expectedMiddleValue : 255;
						const bool isPixelEqual = equalizedImage.GetGrayscalePixel(x, 0) == expectedValue;
						assert(isPixelEqual);
						static_cast<void>(isPixelEqual);
					}
				}
			}
		}
	}

	void TestCreateHistogramEqualizedGrayscaleImageFromConstantImage()
	{
		minicv::Image image(3, 1);
		image.Fill(42);

		const minicv::Image equalizedImage = minicv::CreateHistogramEqualizedGrayscaleImage(image);

		assert(equalizedImage.HasSameContent(image));

		(void)equalizedImage;
	}

	void TestCreateThresholdedGrayscaleImages()
	{
		minicv::Image image(3, 1);
		image.GetGrayscalePixel(0, 0) = 50;
		image.GetGrayscalePixel(1, 0) = 100;
		image.GetGrayscalePixel(2, 0) = 150;

		const minicv::GrayscaleThresholdParameters binaryParameters{ minicv::EThresholdType::BINARY, 100, 200 };
		const minicv::GrayscaleThresholdParameters invertedBinaryParameters{ minicv::EThresholdType::BINARY_INVERTED, 100, 200 };
		const minicv::GrayscaleThresholdParameters truncateParameters{ minicv::EThresholdType::TRUNCATE, 100, 200 };
		const minicv::GrayscaleThresholdParameters toZeroParameters{ minicv::EThresholdType::TO_ZERO, 100, 200 };
		const minicv::GrayscaleThresholdParameters invertedToZeroParameters{ minicv::EThresholdType::TO_ZERO_INVERTED, 100, 200 };

		const minicv::Image binaryImage = minicv::CreateThresholdedGrayscaleImage(image, binaryParameters);
		const minicv::Image invertedBinaryImage = minicv::CreateThresholdedGrayscaleImage(image, invertedBinaryParameters);
		const minicv::Image truncatedImage = minicv::CreateThresholdedGrayscaleImage(image, truncateParameters);
		const minicv::Image toZeroImage = minicv::CreateThresholdedGrayscaleImage(image, toZeroParameters);
		const minicv::Image invertedToZeroImage = minicv::CreateThresholdedGrayscaleImage(image, invertedToZeroParameters);

		AssertGrayscaleRowEquals(binaryImage, std::array<std::uint8_t, 3>{ 0, 0, 200 });
		AssertGrayscaleRowEquals(invertedBinaryImage, std::array<std::uint8_t, 3>{ 200, 200, 0 });
		AssertGrayscaleRowEquals(truncatedImage, std::array<std::uint8_t, 3>{ 50, 100, 100 });
		AssertGrayscaleRowEquals(toZeroImage, std::array<std::uint8_t, 3>{ 0, 0, 150 });
		AssertGrayscaleRowEquals(invertedToZeroImage, std::array<std::uint8_t, 3>{ 50, 100, 0 });

		assert(image.GetGrayscalePixel(2, 0) == 150);
	}

	void TestGrayscaleContrastAndThresholdOperationsFromEmptyImages()
	{
		const minicv::Image emptyImage;
		const minicv::GrayscaleValueRange valueRange{ 50, 200 };
		const minicv::GrayscaleThresholdParameters thresholdParameters{ minicv::EThresholdType::BINARY, 100, 255 };

		const minicv::Image stretchedImage = minicv::CreateContrastStretchedGrayscaleImage(emptyImage, valueRange);
		const minicv::Image equalizedImage = minicv::CreateHistogramEqualizedGrayscaleImage(emptyImage);
		const minicv::Image thresholdedImage = minicv::CreateThresholdedGrayscaleImage(emptyImage, thresholdParameters);

		assert(stretchedImage.IsEmpty());
		assert(equalizedImage.IsEmpty());
		assert(thresholdedImage.IsEmpty());
		assert(stretchedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(equalizedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(thresholdedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);

		(void)stretchedImage;
		(void)equalizedImage;
		(void)thresholdedImage;
	}

	void TestTryCalculateOtsuThreshold()
	{
		minicv::Image image(4, 1);
		image.GetGrayscalePixel(0, 0) = 0;
		image.GetGrayscalePixel(1, 0) = 50;
		image.GetGrayscalePixel(2, 0) = 200;
		image.GetGrayscalePixel(3, 0) = 255;

		const std::optional<std::uint8_t> otsuThreshold = minicv::TryCalculateOtsuThreshold(image);

		assert(otsuThreshold.has_value());
		assert(*otsuThreshold == 50);

		const minicv::GrayscaleThresholdParameters thresholdParameters{ minicv::EThresholdType::BINARY, *otsuThreshold, 255 };
		const minicv::Image thresholdedImage = minicv::CreateThresholdedGrayscaleImage(image, thresholdParameters);

		AssertGrayscaleRowEquals(thresholdedImage, std::array<std::uint8_t, 4>{ 0, 0, 255, 255 });
		assert(image.GetGrayscalePixel(2, 0) == 200);
	}

	void TestTryCalculateOtsuThresholdFromConstantAndEmptyImages()
	{
		minicv::Image constantImage(3, 1);
		constantImage.Fill(42);

		const std::optional<std::uint8_t> constantImageThreshold = minicv::TryCalculateOtsuThreshold(constantImage);
		const std::optional<std::uint8_t> emptyImageThreshold = minicv::TryCalculateOtsuThreshold(minicv::Image{});

		assert(constantImageThreshold.has_value());
		assert(*constantImageThreshold == 0);
		assert(!emptyImageThreshold.has_value());

		(void)constantImageThreshold;
		(void)emptyImageThreshold;
	}

	void TestCreateAdaptiveMeanThresholdedGrayscaleImages()
	{
		minicv::Image image(3, 3);
		image.Fill(10);
		image.GetGrayscalePixel(1, 1) = 200;

		const minicv::GrayscaleAdaptiveThresholdParameters binaryParameters{ minicv::EThresholdType::BINARY, 200, 3, 0.0 };
		const minicv::GrayscaleAdaptiveThresholdParameters invertedBinaryParameters{ minicv::EThresholdType::BINARY_INVERTED, 200, 3, 0.0 };
		const minicv::Image binaryImage = minicv::CreateAdaptiveMeanThresholdedGrayscaleImage(image, binaryParameters);
		const minicv::Image invertedBinaryImage = minicv::CreateAdaptiveMeanThresholdedGrayscaleImage(image, invertedBinaryParameters);

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				const bool isCenterPixel = x == 1 && y == 1;
				const std::uint8_t expectedBinaryValue = isCenterPixel ? 200 : 0;
				const std::uint8_t expectedInvertedBinaryValue = isCenterPixel ? 0 : 200;

				assert(binaryImage.GetGrayscalePixel(x, y) == expectedBinaryValue);
				assert(invertedBinaryImage.GetGrayscalePixel(x, y) == expectedInvertedBinaryValue);

				(void)expectedBinaryValue;
				(void)expectedInvertedBinaryValue;
			}
		}

		assert(image.GetGrayscalePixel(1, 1) == 200);
	}

	void TestCreateAdaptiveMeanThresholdedGrayscaleImageWithMeanOffset()
	{
		minicv::Image image(1, 1);
		image.GetGrayscalePixel(0, 0) = 100;

		const minicv::GrayscaleAdaptiveThresholdParameters zeroOffsetParameters{ minicv::EThresholdType::BINARY, 255, 3, 0.0 };
		const minicv::GrayscaleAdaptiveThresholdParameters positiveOffsetParameters{ minicv::EThresholdType::BINARY, 255, 3, 1.0 };

		const minicv::Image zeroOffsetImage = minicv::CreateAdaptiveMeanThresholdedGrayscaleImage(image, zeroOffsetParameters);
		const minicv::Image positiveOffsetImage = minicv::CreateAdaptiveMeanThresholdedGrayscaleImage(image, positiveOffsetParameters);

		assert(zeroOffsetImage.GetGrayscalePixel(0, 0) == 0);
		assert(positiveOffsetImage.GetGrayscalePixel(0, 0) == 255);
	}

	void TestCreateAdaptiveMeanThresholdedGrayscaleImageAtClippedBoundary()
	{
		minicv::Image image(2, 2);
		image.GetGrayscalePixel(0, 0) = 10;
		image.GetGrayscalePixel(1, 0) = 20;
		image.GetGrayscalePixel(0, 1) = 30;
		image.GetGrayscalePixel(1, 1) = 200;

		const minicv::GrayscaleAdaptiveThresholdParameters thresholdParameters{ minicv::EThresholdType::BINARY, 255, 3, 0.0 };
		const minicv::Image thresholdedImage = minicv::CreateAdaptiveMeanThresholdedGrayscaleImage(image, thresholdParameters);

		assert(thresholdedImage.GetGrayscalePixel(0, 0) == 0);
		assert(thresholdedImage.GetGrayscalePixel(1, 0) == 0);
		assert(thresholdedImage.GetGrayscalePixel(0, 1) == 0);
		assert(thresholdedImage.GetGrayscalePixel(1, 1) == 255);
		assert(image.GetGrayscalePixel(1, 1) == 200);
	}

	void TestCreateAdaptiveMeanThresholdedGrayscaleImageFromEmptyImage()
	{
		const minicv::Image emptyImage;
		const minicv::GrayscaleAdaptiveThresholdParameters thresholdParameters{ minicv::EThresholdType::BINARY, 255, 3, 0.0 };

		const minicv::Image thresholdedImage = minicv::CreateAdaptiveMeanThresholdedGrayscaleImage(emptyImage, thresholdParameters);

		assert(thresholdedImage.IsEmpty());
		assert(thresholdedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);

		(void)thresholdedImage;
	}

	void TestColorAndChannelOperationsFromEmptyImages()
	{
		const minicv::Image emptyRgbImage(0, 0, minicv::EImageType::UINT8_RGB);
		const minicv::Image emptyGrayscaleImage;

		const minicv::Image convertedGrayscaleImage = minicv::ConvertRgbToGrayscale(emptyRgbImage);
		const minicv::Image convertedRgbImage = minicv::ConvertGrayscaleToRgb(emptyGrayscaleImage);
		const minicv::Image swappedImage = minicv::CreateRedBlueChannelSwappedImage(emptyRgbImage);
		const minicv::Image extractedChannelImage = minicv::ExtractRgbChannel(emptyRgbImage, minicv::ERgbChannel::RED);

		minicv::Image redChannelImage;
		minicv::Image greenChannelImage;
		minicv::Image blueChannelImage;

		minicv::SplitRgbChannels(emptyRgbImage, &redChannelImage, &greenChannelImage, &blueChannelImage);
		const minicv::Image mergedImage = minicv::MergeRgbChannels(redChannelImage, greenChannelImage, blueChannelImage);

		assert(convertedGrayscaleImage.IsEmpty());
		assert(convertedGrayscaleImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(convertedRgbImage.IsEmpty());
		assert(convertedRgbImage.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(swappedImage.IsEmpty());
		assert(swappedImage.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(extractedChannelImage.IsEmpty());
		assert(extractedChannelImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(redChannelImage.IsEmpty());
		assert(greenChannelImage.IsEmpty());
		assert(blueChannelImage.IsEmpty());
		assert(mergedImage.IsEmpty());
		assert(mergedImage.GetImageType() == minicv::EImageType::UINT8_RGB);
	}
}

void RunImageOperationsTests()
{
	TestCreateAbsoluteDifferenceImage();
	TestCreateAbsoluteDifferenceRgbImage();
	TestCreateAbsoluteDifferenceImageFromEmptyImages();
	TestCountNonZeroPixels();
	TestCountNonZeroRgbPixels();
	TestCountNonZeroPixelsFromEmptyImage();
	TestGetMaximumPixelDifference();
	TestGetMaximumPixelDifferenceFromEmptyImages();
	TestConvertRgbToGrayscale();
	TestConvertGrayscaleToRgb();
	TestCreateRedBlueChannelSwappedImage();
	TestExtractRgbChannel();
	TestSplitRgbChannels();
	TestSplitRgbChannelsWithSourceAsOutput();
	TestMergeRgbChannels();
	TestSplitAndMergeRgbChannels();
	TestCreateInvertedImage();
	TestAdjustImageBrightness();
	TestAdjustImageBrightnessWithExtremeOffsets();
	TestAdjustImageContrast();
	TestPixelValueOperationsWithRgbImage();
	TestPixelValueOperationsFromEmptyImages();
	TestCalculateGrayscaleHistogram();
	TestCalculateGrayscaleHistogramFromEmptyImage();
	TestCalculateGrayscaleCumulativeDistribution();
	TestCalculateGrayscaleCumulativeDistributionFromEmptyHistogram();
	TestTryGetGrayscaleValueRange();
	TestCreateMinMaxNormalizedGrayscaleImage();
	TestCreateMinMaxNormalizedGrayscaleImageFromConstantImage();
	TestCreateMinMaxNormalizedGrayscaleImageFromEmptyImage();
	TestGrayscaleHistogramAndNormalizationFlow();
	TestCreateContrastStretchedGrayscaleImage();
	TestCreateHistogramEqualizedGrayscaleImage();
	TestHistogramEqualizationRoundsFromPixelCounts();
	TestCreateHistogramEqualizedGrayscaleImageFromConstantImage();
	TestCreateThresholdedGrayscaleImages();
	TestGrayscaleContrastAndThresholdOperationsFromEmptyImages();
	TestTryCalculateOtsuThreshold();
	TestTryCalculateOtsuThresholdFromConstantAndEmptyImages();
	TestCreateAdaptiveMeanThresholdedGrayscaleImages();
	TestCreateAdaptiveMeanThresholdedGrayscaleImageWithMeanOffset();
	TestCreateAdaptiveMeanThresholdedGrayscaleImageAtClippedBoundary();
	TestCreateAdaptiveMeanThresholdedGrayscaleImageFromEmptyImage();
	TestColorAndChannelOperationsFromEmptyImages();
}
