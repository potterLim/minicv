#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "image_test.h"
#include "minicv/Image.h"

namespace
{
	void TestDefaultConstructor()
	{
		const minicv::Image image;

		assert(image.IsEmpty());
		assert(image.GetWidth() == 0);
		assert(image.GetHeight() == 0);
		assert(image.GetChannelCount() == 1);
		assert(image.GetBytesPerRow() == 0);
		assert(image.GetImageType() == minicv::eImageType::UInt8Grayscale);
		assert(image.GetPixelCount() == 0);
		assert(image.GetByteCount() == 0);
	}

	void TestGrayscaleConstructor()
	{
		minicv::Image image(3, 2);

		assert(!image.IsEmpty());
		assert(image.GetWidth() == 3);
		assert(image.GetHeight() == 2);
		assert(image.GetChannelCount() == 1);
		assert(image.GetBytesPerRow() == 3);
		assert(image.GetImageType() == minicv::eImageType::UInt8Grayscale);
		assert(image.GetPixelCount() == 6);
		assert(image.GetByteCount() == 6);

		std::uint8_t* pixelData = image.GetPixelData();
		assert(pixelData != nullptr);

		for (std::size_t index = 0; index < image.GetByteCount(); ++index)
		{
			assert(pixelData[index] == 0);
		}
	}

	void TestRgbConstructor()
	{
		const minicv::Image image(2, 3, minicv::eImageType::UInt8RGB);

		assert(!image.IsEmpty());
		assert(image.GetWidth() == 2);
		assert(image.GetHeight() == 3);
		assert(image.GetChannelCount() == 3);
		assert(image.GetBytesPerRow() == 6);
		assert(image.GetImageType() == minicv::eImageType::UInt8RGB);
		assert(image.GetPixelCount() == 6);
		assert(image.GetByteCount() == 18);
		assert(image.GetPixelData() != nullptr);
	}

	void TestEmptyImages()
	{
		const minicv::Image zeroWidth(0, 5);
		const minicv::Image zeroHeight(5, 0);

		assert(zeroWidth.IsEmpty());
		assert(zeroWidth.GetPixelCount() == 0);
		assert(zeroWidth.GetByteCount() == 0);

		assert(zeroHeight.IsEmpty());
		assert(zeroHeight.GetPixelCount() == 0);
		assert(zeroHeight.GetByteCount() == 0);
	}

	void TestLargeEmptyImages()
	{
		const int maxRgbWidth = std::numeric_limits<int>::max() / 3;
		const minicv::Image wideImage(maxRgbWidth, 0, minicv::eImageType::UInt8RGB);

		assert(wideImage.IsEmpty());
		assert(wideImage.GetWidth() == maxRgbWidth);
		assert(wideImage.GetHeight() == 0);
		assert(wideImage.GetChannelCount() == 3);
		assert(wideImage.GetBytesPerRow() == maxRgbWidth * 3);
		assert(wideImage.GetByteCount() == 0);

		const minicv::Image tallImage(0, std::numeric_limits<int>::max());

		assert(tallImage.IsEmpty());
		assert(tallImage.GetWidth() == 0);
		assert(tallImage.GetHeight() == std::numeric_limits<int>::max());
		assert(tallImage.GetBytesPerRow() == 0);
		assert(tallImage.GetByteCount() == 0);
	}

	void TestGrayscalePixelAccess()
	{
		minicv::Image image(3, 2);

		image.GetGrayscalePixel(2, 1) = 255;

		assert(image.GetGrayscalePixel(2, 1) == 255);
		assert(image.GetPixelData()[5] == 255);

		const minicv::Image& constImage = image;
		assert(constImage.GetGrayscalePixel(2, 1) == 255);
	}

	void TestRgbPixelAccess()
	{
		minicv::Image image(2, 2, minicv::eImageType::UInt8RGB);

		image.GetRgbPixel(1, 1, minicv::eRgbChannel::Red) = 10;
		image.GetRgbPixel(1, 1, minicv::eRgbChannel::Green) = 20;
		image.GetRgbPixel(1, 1, minicv::eRgbChannel::Blue) = 30;

		assert(image.GetRgbPixel(1, 1, minicv::eRgbChannel::Red) == 10);
		assert(image.GetRgbPixel(1, 1, minicv::eRgbChannel::Green) == 20);
		assert(image.GetRgbPixel(1, 1, minicv::eRgbChannel::Blue) == 30);

		assert(image.GetPixelData()[9] == 10);
		assert(image.GetPixelData()[10] == 20);
		assert(image.GetPixelData()[11] == 30);

		const minicv::Image& constImage = image;
		assert(constImage.GetRgbPixel(1, 1, minicv::eRgbChannel::Red) == 10);
		assert(constImage.GetRgbPixel(1, 1, minicv::eRgbChannel::Green) == 20);
		assert(constImage.GetRgbPixel(1, 1, minicv::eRgbChannel::Blue) == 30);
	}
}

void RunImageTests()
{
	TestDefaultConstructor();
	TestGrayscaleConstructor();
	TestRgbConstructor();
	TestEmptyImages();
	TestLargeEmptyImages();
	TestGrayscalePixelAccess();
	TestRgbPixelAccess();
}
