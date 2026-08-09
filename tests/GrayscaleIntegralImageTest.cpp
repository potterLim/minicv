#include <cassert>
#include <limits>

#include "GrayscaleIntegralImageTest.h"
#include "minicv/GrayscaleIntegralImage.h"
#include "minicv/Image.h"

namespace
{
	void TestGrayscaleIntegralImagePropertiesAndRegionSums()
	{
		minicv::Image image(3, 3);
		image.GetGrayscalePixel(0, 0) = 1;
		image.GetGrayscalePixel(1, 0) = 2;
		image.GetGrayscalePixel(2, 0) = 3;
		image.GetGrayscalePixel(0, 1) = 4;
		image.GetGrayscalePixel(1, 1) = 5;
		image.GetGrayscalePixel(2, 1) = 6;
		image.GetGrayscalePixel(0, 2) = 7;
		image.GetGrayscalePixel(1, 2) = 8;
		image.GetGrayscalePixel(2, 2) = 9;

		const minicv::GrayscaleIntegralImage integralImage(image);

		assert(!integralImage.IsEmpty());
		assert(integralImage.GetWidth() == 3);
		assert(integralImage.GetHeight() == 3);
		assert(integralImage.GetSize().Width == 3);
		assert(integralImage.GetSize().Height == 3);
		assert(integralImage.GetRegionSum(minicv::Rect{ 0, 0, 3, 3 }) == 45);
		assert(integralImage.GetRegionSum(minicv::Rect{ 0, 0, 2, 2 }) == 12);
		assert(integralImage.GetRegionSum(minicv::Rect{ 1, 1, 2, 2 }) == 28);
		assert(integralImage.GetRegionSum(minicv::Rect{ 2, 0, 1, 1 }) == 3);
		assert(integralImage.GetRegionSum(minicv::Rect{ 1, 1, 0, 2 }) == 0);
		assert(image.GetGrayscalePixel(1, 1) == 5);
	}

	void TestGrayscaleIntegralImageMaximumPixelValues()
	{
		minicv::Image image(2, 2);
		image.Fill(255);

		const minicv::GrayscaleIntegralImage integralImage(image);

		assert(integralImage.GetRegionSum(minicv::Rect{ 0, 0, 2, 2 }) == 1'020);
		assert(integralImage.GetRegionSum(minicv::Rect{ 1, 0, 1, 2 }) == 510);
	}

	void TestGrayscaleIntegralImageFromEmptyImages()
	{
		const minicv::Image emptyImage;
		const minicv::Image zeroWidthImage(0, 5);
		const minicv::Image zeroHeightImage(std::numeric_limits<int>::max(), 0);

		const minicv::GrayscaleIntegralImage emptyIntegralImage(emptyImage);
		const minicv::GrayscaleIntegralImage zeroWidthIntegralImage(zeroWidthImage);
		const minicv::GrayscaleIntegralImage zeroHeightIntegralImage(zeroHeightImage);

		assert(emptyIntegralImage.IsEmpty());
		assert(emptyIntegralImage.GetRegionSum(minicv::Rect{ 0, 0, 0, 0 }) == 0);
		assert(zeroWidthIntegralImage.IsEmpty());
		assert(zeroWidthIntegralImage.GetWidth() == 0);
		assert(zeroWidthIntegralImage.GetHeight() == 5);
		assert(zeroWidthIntegralImage.GetRegionSum(minicv::Rect{ 0, 0, 0, 5 }) == 0);
		assert(zeroHeightIntegralImage.IsEmpty());
		assert(zeroHeightIntegralImage.GetWidth() == std::numeric_limits<int>::max());
		assert(zeroHeightIntegralImage.GetHeight() == 0);
		assert(zeroHeightIntegralImage.GetRegionSum(minicv::Rect{ 0, 0, std::numeric_limits<int>::max(), 0 }) == 0);
	}
}

void RunGrayscaleIntegralImageTests()
{
	TestGrayscaleIntegralImagePropertiesAndRegionSums();
	TestGrayscaleIntegralImageMaximumPixelValues();
	TestGrayscaleIntegralImageFromEmptyImages();
}
