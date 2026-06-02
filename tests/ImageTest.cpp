#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "ImageTest.h"
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
		assert(image.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
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
		assert(image.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(image.GetPixelCount() == 6);
		assert(image.GetByteCount() == 6);

		assert(image.GetPixelData() != nullptr);

		for (std::size_t index = 0; index < image.GetByteCount(); ++index)
		{
			assert(image.GetPixelData()[index] == 0);
		}
	}

	void TestRgbConstructor()
	{
		const minicv::Image image(2, 3, minicv::EImageType::UINT8_RGB);

		assert(!image.IsEmpty());
		assert(image.GetWidth() == 2);
		assert(image.GetHeight() == 3);
		assert(image.GetChannelCount() == 3);
		assert(image.GetBytesPerRow() == 6);
		assert(image.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(image.GetPixelCount() == 6);
		assert(image.GetByteCount() == 18);
		assert(image.GetPixelData() != nullptr);
	}

	void TestSizeConstructor()
	{
		const minicv::Size size{ 4, 3 };
		const minicv::Image image(size, minicv::EImageType::UINT8_RGB);

		assert(image.GetWidth() == 4);
		assert(image.GetHeight() == 3);
		assert(image.GetSize().Width == 4);
		assert(image.GetSize().Height == 3);
		assert(image.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(image.GetChannelCount() == 3);
		assert(image.GetByteCount() == 36);
	}

	void TestCreate()
	{
		minicv::Image image(1, 1, minicv::EImageType::UINT8_RGB);
		image.FillRgb(10, 20, 30);

		image.Create(3, 2);

		assert(image.GetWidth() == 3);
		assert(image.GetHeight() == 2);
		assert(image.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(image.GetChannelCount() == 1);
		assert(image.GetByteCount() == 6);

		for (std::size_t index = 0; index < image.GetByteCount(); ++index)
		{
			assert(image.GetPixelData()[index] == 0);
		}

		const minicv::Size size{ 2, 2 };
		image.Create(size, minicv::EImageType::UINT8_RGB);

		assert(image.GetWidth() == 2);
		assert(image.GetHeight() == 2);
		assert(image.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(image.GetChannelCount() == 3);
		assert(image.GetByteCount() == 12);
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
		const minicv::Image wideImage(maxRgbWidth, 0, minicv::EImageType::UINT8_RGB);

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

		assert(static_cast<const minicv::Image&>(image).GetGrayscalePixel(2, 1) == 255);
	}

	void TestRgbPixelAccess()
	{
		minicv::Image image(2, 2, minicv::EImageType::UINT8_RGB);

		image.GetRgbPixel(1, 1, minicv::ERgbChannel::RED) = 10;
		image.GetRgbPixel(1, 1, minicv::ERgbChannel::GREEN) = 20;
		image.GetRgbPixel(1, 1, minicv::ERgbChannel::BLUE) = 30;

		assert(image.GetRgbPixel(1, 1, minicv::ERgbChannel::RED) == 10);
		assert(image.GetRgbPixel(1, 1, minicv::ERgbChannel::GREEN) == 20);
		assert(image.GetRgbPixel(1, 1, minicv::ERgbChannel::BLUE) == 30);

		assert(image.GetPixelData()[9] == 10);
		assert(image.GetPixelData()[10] == 20);
		assert(image.GetPixelData()[11] == 30);

		assert(static_cast<const minicv::Image&>(image).GetRgbPixel(1, 1, minicv::ERgbChannel::RED) == 10);
		assert(static_cast<const minicv::Image&>(image).GetRgbPixel(1, 1, minicv::ERgbChannel::GREEN) == 20);
		assert(static_cast<const minicv::Image&>(image).GetRgbPixel(1, 1, minicv::ERgbChannel::BLUE) == 30);
	}

	void TestFill()
	{
		minicv::Image image(3, 2);

		image.Fill(42);

		for (std::size_t index = 0; index < image.GetByteCount(); ++index)
		{
			assert(image.GetPixelData()[index] == 42);
		}
	}

	void TestFillRgb()
	{
		minicv::Image image(2, 2, minicv::EImageType::UINT8_RGB);

		image.FillRgb(10, 20, 30);

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				assert(image.GetRgbPixel(x, y, minicv::ERgbChannel::RED) == 10);
				assert(image.GetRgbPixel(x, y, minicv::ERgbChannel::GREEN) == 20);
				assert(image.GetRgbPixel(x, y, minicv::ERgbChannel::BLUE) == 30);
			}
		}
	}

	void TestSizeAndShapeComparison()
	{
		const minicv::Image grayscaleImage(3, 2);
		const minicv::Image sameShapeImage(3, 2);
		const minicv::Image sameSizeRgbImage(3, 2, minicv::EImageType::UINT8_RGB);
		const minicv::Image differentSizeImage(2, 3);

		assert(grayscaleImage.HasSameSize(sameShapeImage));
		assert(grayscaleImage.HasSameShape(sameShapeImage));

		assert(grayscaleImage.HasSameSize(sameSizeRgbImage));
		assert(!grayscaleImage.HasSameShape(sameSizeRgbImage));

		assert(!grayscaleImage.HasSameSize(differentSizeImage));
		assert(!grayscaleImage.HasSameShape(differentSizeImage));
	}

	void TestContainsPoint()
	{
		const minicv::Image image(3, 2);

		assert(image.Contains(minicv::Point{ 0, 0 }));
		assert(image.Contains(minicv::Point{ 2, 1 }));
		assert(!image.Contains(minicv::Point{ -1, 0 }));
		assert(!image.Contains(minicv::Point{ 3, 0 }));
		assert(!image.Contains(minicv::Point{ 0, 2 }));
	}

	void TestContainsRect()
	{
		const minicv::Image image(4, 3);

		assert(image.Contains(minicv::Rect{ 0, 0, 4, 3 }));
		assert(image.Contains(minicv::Rect{ 1, 1, 2, 1 }));
		assert(image.Contains(minicv::Rect{ 4, 3, 0, 0 }));
		assert(!image.Contains(minicv::Rect{ -1, 0, 1, 1 }));
		assert(!image.Contains(minicv::Rect{ 0, -1, 1, 1 }));
		assert(!image.Contains(minicv::Rect{ 0, 0, -1, 1 }));
		assert(!image.Contains(minicv::Rect{ 0, 0, 1, -1 }));
		assert(!image.Contains(minicv::Rect{ 3, 0, 2, 1 }));
		assert(!image.Contains(minicv::Rect{ 0, 2, 1, 2 }));
	}

	void TestClone()
	{
		minicv::Image image(2, 2);
		image.GetGrayscalePixel(1, 1) = 77;

		minicv::Image clonedImage = image.Clone();
		image.GetGrayscalePixel(1, 1) = 11;

		assert(clonedImage.GetWidth() == 2);
		assert(clonedImage.GetHeight() == 2);
		assert(clonedImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(clonedImage.GetGrayscalePixel(1, 1) == 77);
		assert(image.GetGrayscalePixel(1, 1) == 11);
	}

	void TestExtractGrayscaleRegion()
	{
		minicv::Image image(4, 3);

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				image.GetGrayscalePixel(x, y) = static_cast<std::uint8_t>(y * 10 + x);
			}
		}

		minicv::Image region = image.ExtractRegion(minicv::Rect{ 1, 1, 2, 2 });

		assert(region.GetWidth() == 2);
		assert(region.GetHeight() == 2);
		assert(region.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(region.GetGrayscalePixel(0, 0) == 11);
		assert(region.GetGrayscalePixel(1, 0) == 12);
		assert(region.GetGrayscalePixel(0, 1) == 21);
		assert(region.GetGrayscalePixel(1, 1) == 22);

		image.GetGrayscalePixel(1, 1) = 99;
		assert(region.GetGrayscalePixel(0, 0) == 11);
	}

	void TestExtractRgbRegion()
	{
		minicv::Image image(3, 2, minicv::EImageType::UINT8_RGB);

		image.GetRgbPixel(1, 0, minicv::ERgbChannel::RED) = 10;
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::GREEN) = 20;
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::BLUE) = 30;
		image.GetRgbPixel(2, 0, minicv::ERgbChannel::RED) = 40;
		image.GetRgbPixel(2, 0, minicv::ERgbChannel::GREEN) = 50;
		image.GetRgbPixel(2, 0, minicv::ERgbChannel::BLUE) = 60;

		minicv::Image region = image.ExtractRegion(minicv::Rect{ 1, 0, 2, 1 });

		assert(region.GetWidth() == 2);
		assert(region.GetHeight() == 1);
		assert(region.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(region.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) == 10);
		assert(region.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) == 20);
		assert(region.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) == 30);
		assert(region.GetRgbPixel(1, 0, minicv::ERgbChannel::RED) == 40);
		assert(region.GetRgbPixel(1, 0, minicv::ERgbChannel::GREEN) == 50);
		assert(region.GetRgbPixel(1, 0, minicv::ERgbChannel::BLUE) == 60);
	}
}

void RunImageTests()
{
	TestDefaultConstructor();
	TestGrayscaleConstructor();
	TestRgbConstructor();
	TestSizeConstructor();
	TestCreate();
	TestEmptyImages();
	TestLargeEmptyImages();
	TestGrayscalePixelAccess();
	TestRgbPixelAccess();
	TestFill();
	TestFillRgb();
	TestSizeAndShapeComparison();
	TestContainsPoint();
	TestContainsRect();
	TestClone();
	TestExtractGrayscaleRegion();
	TestExtractRgbRegion();
}
