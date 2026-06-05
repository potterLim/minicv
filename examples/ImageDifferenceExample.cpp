#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>

#include "minicv/Image.h"
#include "minicv/ImageOperations.h"
#include "minicv/Rect.h"

namespace
{
	constexpr int FRAME_WIDTH = 8;
	constexpr int FRAME_HEIGHT = 6;
	constexpr std::uint8_t BACKGROUND_VALUE = 0;
	constexpr std::uint8_t OBJECT_VALUE = 200;
	constexpr std::size_t EXPECTED_CHANGED_PIXEL_COUNT = 8;

	void DrawFilledRectangle(minicv::Image& image, const minicv::Rect region, const std::uint8_t pixelValue)
	{
		assert(image.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");
		assert(image.Contains(region) && "region must be inside the image.");

		for (int y = region.Y; y < region.Y + region.Height; ++y)
		{
			for (int x = region.X; x < region.X + region.Width; ++x)
			{
				image.GetGrayscalePixel(x, y) = pixelValue;
			}
		}
	}

	void PrintGrayscaleImage(const minicv::Image& image)
	{
		assert(image.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				std::cout << static_cast<int>(image.GetGrayscalePixel(x, y)) << ' ';
			}

			std::cout << '\n';
		}
	}
}

int main()
{
	const minicv::Rect previousObjectRegion{ 1, 2, 3, 2 };
	const minicv::Rect currentObjectRegion{ 3, 2, 3, 2 };
	const minicv::Rect changedRegion{ 1, 2, 5, 2 };

	minicv::Image previousFrame(FRAME_WIDTH, FRAME_HEIGHT);
	previousFrame.Fill(BACKGROUND_VALUE);
	DrawFilledRectangle(previousFrame, previousObjectRegion, OBJECT_VALUE);

	minicv::Image currentFrame = previousFrame.Clone();
	DrawFilledRectangle(currentFrame, previousObjectRegion, BACKGROUND_VALUE);
	DrawFilledRectangle(currentFrame, currentObjectRegion, OBJECT_VALUE);

	const bool hasChanged = !previousFrame.HasSameContent(currentFrame);
	const minicv::Image differenceImage = minicv::CreateAbsoluteDifferenceImage(previousFrame, currentFrame);
	const std::size_t changedPixelCount = minicv::CountNonZeroPixels(differenceImage);
	const std::uint8_t maximumPixelDifference = minicv::GetMaximumPixelDifference(previousFrame, currentFrame);
	const minicv::Image changedRegionImage = differenceImage.ExtractRegion(changedRegion);

	if (!hasChanged || changedPixelCount != EXPECTED_CHANGED_PIXEL_COUNT || maximumPixelDifference != OBJECT_VALUE)
	{
		std::cerr << "Unexpected image difference result.\n";
		return 1;
	}

	std::cout << "Image difference example\n";
	std::cout << "Changed: " << hasChanged << '\n';
	std::cout << "Changed pixel count: " << changedPixelCount << '\n';
	std::cout << "Maximum pixel difference: " << static_cast<int>(maximumPixelDifference) << '\n';
	std::cout << "\nChanged region difference image:\n";
	PrintGrayscaleImage(changedRegionImage);

	return 0;
}
