#include <cassert>

#include "ImageOperationsTest.h"
#include "minicv/Image.h"
#include "minicv/ImageOperations.h"

namespace
{
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
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::GREEN) = 0;
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::BLUE) = 0;

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
}

void RunImageOperationsTests()
{
	TestCreateAbsoluteDifferenceImage();
	TestCreateAbsoluteDifferenceRgbImage();
	TestCountNonZeroPixels();
	TestCountNonZeroRgbPixels();
	TestCountNonZeroPixelsFromEmptyImage();
	TestGetMaximumPixelDifference();
	TestGetMaximumPixelDifferenceFromEmptyImages();
}
