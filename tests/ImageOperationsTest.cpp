#include <cassert>
#include <cstdint>

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
	TestAdjustImageContrast();
	TestPixelValueOperationsWithRgbImage();
	TestPixelValueOperationsFromEmptyImages();
	TestColorAndChannelOperationsFromEmptyImages();
}
