#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "ImageMorphologyTest.h"
#include "minicv/ImageMorphology.h"

namespace
{
	template <std::size_t BYTE_COUNT>
	void AssertPixelsEqual(const minicv::Image& image, const std::array<std::uint8_t, BYTE_COUNT>& expectedPixels)
	{
		assert(image.GetByteCount() == BYTE_COUNT);
		for (std::size_t pixelIndex = 0; pixelIndex < BYTE_COUNT; ++pixelIndex)
		{
			const bool isPixelEqual = image.GetPixelData()[pixelIndex] == expectedPixels[pixelIndex];
			assert(isPixelEqual);
			static_cast<void>(isPixelEqual);
		}
	}

	template <std::size_t BYTE_COUNT>
	minicv::Image CreateImage(
		const minicv::Size size,
		const minicv::EImageType imageType,
		const std::array<std::uint8_t, BYTE_COUNT>& pixels)
	{
		minicv::Image image(size, imageType);
		assert(image.GetByteCount() == BYTE_COUNT);
		for (std::size_t pixelIndex = 0; pixelIndex < BYTE_COUNT; ++pixelIndex)
		{
			image.GetPixelData()[pixelIndex] = pixels[pixelIndex];
		}
		return image;
	}

	void TestErosionAndDilationUseGrayscaleExtrema()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 5, 1 }, minicv::EImageType::UINT8_GRAYSCALE,
			std::array<std::uint8_t, 5>{ 10, 80, 30, 200, 50 });
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 1 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 255 };

		AssertPixelsEqual(minicv::CreateErodedImage(image, element, borderParameters),
			std::array<std::uint8_t, 5>{ 10, 10, 30, 30, 50 });
		AssertPixelsEqual(minicv::CreateDilatedImage(image, element, borderParameters),
			std::array<std::uint8_t, 5>{ 80, 80, 200, 200, 200 });
		AssertPixelsEqual(image, std::array<std::uint8_t, 5>{ 10, 80, 30, 200, 50 });
	}

	void TestCrossMaskExcludesDiagonalNeighbors()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 3, 3 }, minicv::EImageType::UINT8_GRAYSCALE,
			std::array<std::uint8_t, 9>{ 10, 20, 30, 40, 50, 60, 70, 80, 90 });
		const minicv::StructuringElement cross = minicv::CreateCrossStructuringElement(minicv::Size{ 3, 3 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };

		AssertPixelsEqual(minicv::CreateErodedImage(image, cross, borderParameters),
			std::array<std::uint8_t, 9>{ 10, 10, 20, 10, 20, 30, 40, 50, 60 });
		AssertPixelsEqual(minicv::CreateDilatedImage(image, cross, borderParameters),
			std::array<std::uint8_t, 9>{ 40, 50, 60, 70, 80, 90, 80, 90, 90 });
	}

	void TestAsymmetricMaskReflectsOnlyForDilation()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 3, 2 }, minicv::EImageType::UINT8_GRAYSCALE,
			std::array<std::uint8_t, 6>{ 10, 20, 30, 40, 50, 60 });
		const minicv::StructuringElement element(minicv::Size{ 3, 3 }, std::vector<std::uint8_t>{ 7, 0, 0, 0, 0, 0, 0, 0, 0 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 99 };

		AssertPixelsEqual(minicv::CreateErodedImage(image, element, borderParameters),
			std::array<std::uint8_t, 6>{ 99, 99, 99, 99, 10, 20 });
		AssertPixelsEqual(minicv::CreateDilatedImage(image, element, borderParameters),
			std::array<std::uint8_t, 6>{ 50, 60, 99, 99, 99, 99 });
	}

	void TestLargeMaskAndConstantBordersHandleSinglePixel()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 1, 1 }, minicv::EImageType::UINT8_GRAYSCALE,
			std::array<std::uint8_t, 1>{ 90 });
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 5, 3 });
		const minicv::ImageBorderParameters lowBorder{ minicv::EBorderType::CONSTANT, 15 };
		const minicv::ImageBorderParameters highBorder{ minicv::EBorderType::CONSTANT, 200 };
		const minicv::ImageBorderParameters replicateBorder{ minicv::EBorderType::REPLICATE, 0 };

		AssertPixelsEqual(minicv::CreateErodedImage(image, element, lowBorder), std::array<std::uint8_t, 1>{ 15 });
		AssertPixelsEqual(minicv::CreateDilatedImage(image, element, lowBorder), std::array<std::uint8_t, 1>{ 90 });
		AssertPixelsEqual(minicv::CreateErodedImage(image, element, highBorder), std::array<std::uint8_t, 1>{ 90 });
		AssertPixelsEqual(minicv::CreateDilatedImage(image, element, highBorder), std::array<std::uint8_t, 1>{ 200 });
		AssertPixelsEqual(minicv::CreateErodedImage(image, element, replicateBorder), std::array<std::uint8_t, 1>{ 90 });
		AssertPixelsEqual(minicv::CreateDilatedImage(image, element, replicateBorder), std::array<std::uint8_t, 1>{ 90 });
	}

	void TestRgbChannelsAreProcessedIndependently()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 3, 1 }, minicv::EImageType::UINT8_RGB,
			std::array<std::uint8_t, 9>{ 10, 200, 30, 100, 20, 220, 50, 150, 60 });
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 1 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::Image erodedImage = minicv::CreateErodedImage(image, element, borderParameters);
		const minicv::Image dilatedImage = minicv::CreateDilatedImage(image, element, borderParameters);

		assert(erodedImage.HasSameShape(image));
		assert(dilatedImage.HasSameShape(image));
		AssertPixelsEqual(erodedImage, std::array<std::uint8_t, 9>{ 10, 20, 30, 10, 20, 30, 50, 20, 60 });
		AssertPixelsEqual(dilatedImage, std::array<std::uint8_t, 9>{ 100, 200, 220, 100, 200, 220, 100, 150, 220 });
		AssertPixelsEqual(image, std::array<std::uint8_t, 9>{ 10, 200, 30, 100, 20, 220, 50, 150, 60 });
	}

	void TestOpeningRemovesIsolatedBrightPixel()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 9, 1 }, minicv::EImageType::UINT8_GRAYSCALE,
			std::array<std::uint8_t, 9>{ 0, 255, 0, 0, 255, 255, 255, 0, 0 });
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 1 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };

		AssertPixelsEqual(minicv::CreateOpenedImage(image, element, borderParameters),
			std::array<std::uint8_t, 9>{ 0, 0, 0, 0, 255, 255, 255, 0, 0 });
		AssertPixelsEqual(image, std::array<std::uint8_t, 9>{ 0, 255, 0, 0, 255, 255, 255, 0, 0 });
	}

	void TestClosingFillsNarrowDarkGap()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 1, 9 }, minicv::EImageType::UINT8_GRAYSCALE,
			std::array<std::uint8_t, 9>{ 0, 0, 255, 255, 0, 255, 255, 0, 0 });
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 1, 3 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };

		AssertPixelsEqual(minicv::CreateClosedImage(image, element, borderParameters),
			std::array<std::uint8_t, 9>{ 0, 0, 255, 255, 255, 255, 255, 0, 0 });
		AssertPixelsEqual(image, std::array<std::uint8_t, 9>{ 0, 0, 255, 255, 0, 255, 255, 0, 0 });
	}

	void TestCompoundOperationsReuseConstantBorderForIntermediateImage()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 1, 1 }, minicv::EImageType::UINT8_RGB,
			std::array<std::uint8_t, 3>{ 90, 100, 110 });
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 3 });
		const minicv::ImageBorderParameters lowBorder{ minicv::EBorderType::CONSTANT, 20 };
		const minicv::ImageBorderParameters highBorder{ minicv::EBorderType::CONSTANT, 200 };

		AssertPixelsEqual(minicv::CreateOpenedImage(image, element, lowBorder), std::array<std::uint8_t, 3>{ 20, 20, 20 });
		AssertPixelsEqual(minicv::CreateClosedImage(image, element, lowBorder), std::array<std::uint8_t, 3>{ 20, 20, 20 });
		AssertPixelsEqual(minicv::CreateOpenedImage(image, element, highBorder), std::array<std::uint8_t, 3>{ 200, 200, 200 });
		AssertPixelsEqual(minicv::CreateClosedImage(image, element, highBorder), std::array<std::uint8_t, 3>{ 200, 200, 200 });
	}

	void TestSinglePositionMaskPreservesPixelsAndShape()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 2, 1 }, minicv::EImageType::UINT8_RGB,
			std::array<std::uint8_t, 6>{ 0, 128, 255, 200, 50, 90 });
		const minicv::StructuringElement element = minicv::CreateCrossStructuringElement(minicv::Size{ 1, 1 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 0 };
		const std::array<minicv::Image, 4> processedImages{
			minicv::CreateErodedImage(image, element, borderParameters),
			minicv::CreateDilatedImage(image, element, borderParameters),
			minicv::CreateOpenedImage(image, element, borderParameters),
			minicv::CreateClosedImage(image, element, borderParameters)
		};
		for (const minicv::Image& processedImage : processedImages)
		{
			const bool isContentEqual = processedImage.HasSameContent(image);
			assert(isContentEqual);
			static_cast<void>(isContentEqual);
		}
	}

	void TestMorphologyPreservesEmptyShapesAndTypes()
	{
		const std::array<minicv::Size, 3> sizes{ minicv::Size{ 0, 0 }, minicv::Size{ 0, 4 }, minicv::Size{ 3, 0 } };
		const std::array<minicv::EImageType, 2> imageTypes{ minicv::EImageType::UINT8_GRAYSCALE, minicv::EImageType::UINT8_RGB };
		const std::array<minicv::ImageBorderParameters, 2> borders{
			minicv::ImageBorderParameters{ minicv::EBorderType::CONSTANT, 20 },
			minicv::ImageBorderParameters{ minicv::EBorderType::REPLICATE, 0 }
		};
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 3 });
		for (const minicv::Size size : sizes)
		{
			for (const minicv::EImageType imageType : imageTypes)
			{
				const minicv::Image image(size, imageType);
				for (const minicv::ImageBorderParameters border : borders)
				{
					const std::array<minicv::Image, 4> processedImages{
						minicv::CreateErodedImage(image, element, border),
						minicv::CreateDilatedImage(image, element, border),
						minicv::CreateOpenedImage(image, element, border),
						minicv::CreateClosedImage(image, element, border)
					};
					for (const minicv::Image& processedImage : processedImages)
					{
						const bool isEmptyShapePreserved = processedImage.IsEmpty() && processedImage.HasSameShape(image);
						assert(isEmptyShapePreserved);
						static_cast<void>(isEmptyShapePreserved);
					}
				}
			}
		}
	}
}

void RunImageMorphologyTests()
{
	TestErosionAndDilationUseGrayscaleExtrema();
	TestCrossMaskExcludesDiagonalNeighbors();
	TestAsymmetricMaskReflectsOnlyForDilation();
	TestLargeMaskAndConstantBordersHandleSinglePixel();
	TestRgbChannelsAreProcessedIndependently();
	TestOpeningRemovesIsolatedBrightPixel();
	TestClosingFillsNarrowDarkGap();
	TestCompoundOperationsReuseConstantBorderForIntermediateImage();
	TestSinglePositionMaskPreservesPixelsAndShape();
	TestMorphologyPreservesEmptyShapesAndTypes();
}
