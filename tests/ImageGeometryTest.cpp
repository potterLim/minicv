#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "ImageGeometryTest.h"
#include "minicv/ImageGeometry.h"

namespace
{
	template <std::size_t BYTE_COUNT>
	minicv::Image CreateImage(const minicv::Size size, const minicv::EImageType imageType, const std::array<std::uint8_t, BYTE_COUNT>& pixels)
	{
		minicv::Image image(size, imageType);
		assert(image.GetByteCount() == BYTE_COUNT);
		std::copy(pixels.begin(), pixels.end(), image.GetPixelData());
		return image;
	}

	template <std::size_t BYTE_COUNT>
	void AssertPixelsEqual(const minicv::Image& image, const std::array<std::uint8_t, BYTE_COUNT>& expectedPixels)
	{
		assert(image.GetByteCount() == BYTE_COUNT);
		for (std::size_t index = 0; index < BYTE_COUNT; ++index)
		{
			assert(image.GetPixelData()[index] == expectedPixels[index]);
		}
	}

	void TestNearestResizeUsesFloorCoordinates()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 3, 2 }, minicv::EImageType::UINT8_GRAYSCALE, std::array<std::uint8_t, 6>{ 0, 30, 60, 90, 120, 150 });
		const minicv::Image enlarged = minicv::CreateResizedImage(image, minicv::Size{ 5, 3 }, minicv::EInterpolationType::NEAREST_NEIGHBOR);
		assert(enlarged.GetWidth() == 5 && enlarged.GetHeight() == 3);
		AssertPixelsEqual(enlarged, std::array<std::uint8_t, 15>
		{
			0, 0, 30, 30, 60,
			0, 0, 30, 30, 60,
			90, 90, 120, 120, 150
		});

		const minicv::Image reduced = minicv::CreateResizedImage(image, minicv::Size{ 2, 1 }, minicv::EInterpolationType::NEAREST_NEIGHBOR);
		AssertPixelsEqual(reduced, std::array<std::uint8_t, 2>{ 0, 30 });
	}

	void TestBilinearResizeUsesPixelCentersAndClampedEdges()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 2, 2 }, minicv::EImageType::UINT8_GRAYSCALE, std::array<std::uint8_t, 4>{ 0, 40, 80, 120 });
		const minicv::Image enlarged = minicv::CreateResizedImage(image, minicv::Size{ 3, 3 }, minicv::EInterpolationType::BILINEAR);
		AssertPixelsEqual(enlarged, std::array<std::uint8_t, 9>
		{
			0, 20, 40,
			40, 60, 80,
			80, 100, 120
		});

		const minicv::Image reduced = minicv::CreateResizedImage(image, minicv::Size{ 1, 1 }, minicv::EInterpolationType::BILINEAR);
		AssertPixelsEqual(reduced, std::array<std::uint8_t, 1>{ 60 });
	}

	void TestBilinearResizeHandlesSingleRowsAndColumns()
	{
		const minicv::Image row = CreateImage(minicv::Size{ 2, 1 }, minicv::EImageType::UINT8_GRAYSCALE, std::array<std::uint8_t, 2>{ 0, 255 });
		const minicv::Image column = CreateImage(minicv::Size{ 1, 2 }, minicv::EImageType::UINT8_GRAYSCALE, std::array<std::uint8_t, 2>{ 0, 255 });
		const minicv::Image enlargedRow = minicv::CreateResizedImage(row, minicv::Size{ 4, 1 }, minicv::EInterpolationType::BILINEAR);
		const minicv::Image enlargedColumn = minicv::CreateResizedImage(column, minicv::Size{ 1, 4 }, minicv::EInterpolationType::BILINEAR);
		AssertPixelsEqual(enlargedRow, std::array<std::uint8_t, 4>{ 0, 64, 191, 255 });
		AssertPixelsEqual(enlargedColumn, std::array<std::uint8_t, 4>{ 0, 64, 191, 255 });

		const minicv::Image wideRow = CreateImage(minicv::Size{ 3, 1 }, minicv::EImageType::UINT8_GRAYSCALE, std::array<std::uint8_t, 3>{ 0, 40, 80 });
		const minicv::Image reducedRow = minicv::CreateResizedImage(wideRow, minicv::Size{ 2, 1 }, minicv::EInterpolationType::BILINEAR);
		AssertPixelsEqual(reducedRow, std::array<std::uint8_t, 2>{ 10, 70 });
	}

	void TestBilinearResizeRoundsHalfUp()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 2, 1 }, minicv::EImageType::UINT8_GRAYSCALE, std::array<std::uint8_t, 2>{ 0, 1 });
		const minicv::Image resized = minicv::CreateResizedImage(image, minicv::Size{ 3, 1 }, minicv::EInterpolationType::BILINEAR);
		AssertPixelsEqual(resized, std::array<std::uint8_t, 3>{ 0, 1, 1 });
	}

	void TestResizePreservesRgbChannelOrder()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 2, 1 }, minicv::EImageType::UINT8_RGB, std::array<std::uint8_t, 6>{ 0, 100, 255, 100, 200, 0 });
		const minicv::Image nearest = minicv::CreateResizedImage(image, minicv::Size{ 3, 1 }, minicv::EInterpolationType::NEAREST_NEIGHBOR);
		const minicv::Image bilinear = minicv::CreateResizedImage(image, minicv::Size{ 3, 1 }, minicv::EInterpolationType::BILINEAR);
		assert(nearest.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(bilinear.GetImageType() == minicv::EImageType::UINT8_RGB);
		AssertPixelsEqual(nearest, std::array<std::uint8_t, 9>{ 0, 100, 255, 0, 100, 255, 100, 200, 0 });
		AssertPixelsEqual(bilinear, std::array<std::uint8_t, 9>{ 0, 100, 255, 50, 150, 128, 100, 200, 0 });
	}

	void TestFlipsAndRotationsUseImageCoordinates()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 3, 2 }, minicv::EImageType::UINT8_GRAYSCALE, std::array<std::uint8_t, 6>{ 1, 2, 3, 4, 5, 6 });
		const minicv::Image horizontal = minicv::CreateHorizontallyFlippedImage(image);
		const minicv::Image vertical = minicv::CreateVerticallyFlippedImage(image);
		const minicv::Image clockwise = minicv::CreateRotated90Image(image, minicv::ERotationDirection::CLOCKWISE);
		const minicv::Image counterclockwise = minicv::CreateRotated90Image(image, minicv::ERotationDirection::COUNTERCLOCKWISE);
		assert(horizontal.HasSameShape(image) && vertical.HasSameShape(image));
		assert(clockwise.GetWidth() == 2 && clockwise.GetHeight() == 3);
		assert(counterclockwise.HasSameShape(clockwise));
		AssertPixelsEqual(horizontal, std::array<std::uint8_t, 6>{ 3, 2, 1, 6, 5, 4 });
		AssertPixelsEqual(vertical, std::array<std::uint8_t, 6>{ 4, 5, 6, 1, 2, 3 });
		AssertPixelsEqual(clockwise, std::array<std::uint8_t, 6>
		{
			4, 1,
			5, 2,
			6, 3
		});
		AssertPixelsEqual(counterclockwise, std::array<std::uint8_t, 6>
		{
			3, 6,
			2, 5,
			1, 4
		});
	}

	void TestRgbFlipsAndRotationsCopyWholePixels()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 2, 1 }, minicv::EImageType::UINT8_RGB, std::array<std::uint8_t, 6>{ 10, 20, 30, 40, 50, 60 });
		const minicv::Image horizontal = minicv::CreateHorizontallyFlippedImage(image);
		const minicv::Image clockwise = minicv::CreateRotated90Image(image, minicv::ERotationDirection::CLOCKWISE);
		const minicv::Image counterclockwise = minicv::CreateRotated90Image(image, minicv::ERotationDirection::COUNTERCLOCKWISE);
		assert(horizontal.HasSameShape(image));
		assert(clockwise.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(clockwise.GetWidth() == 1 && clockwise.GetHeight() == 2);
		AssertPixelsEqual(horizontal, std::array<std::uint8_t, 6>{ 40, 50, 60, 10, 20, 30 });
		AssertPixelsEqual(clockwise, std::array<std::uint8_t, 6>{ 10, 20, 30, 40, 50, 60 });
		AssertPixelsEqual(counterclockwise, std::array<std::uint8_t, 6>{ 40, 50, 60, 10, 20, 30 });
		AssertPixelsEqual(minicv::CreateVerticallyFlippedImage(clockwise), std::array<std::uint8_t, 6>{ 40, 50, 60, 10, 20, 30 });
	}

	void TestIdentityResizeAndSinglePixelExpansionOwnTheirPixels()
	{
		const std::array<minicv::EInterpolationType, 2> interpolations{ minicv::EInterpolationType::NEAREST_NEIGHBOR, minicv::EInterpolationType::BILINEAR };
		for (const minicv::EInterpolationType interpolation : interpolations)
		{
			minicv::Image image = CreateImage(minicv::Size{ 3, 1 }, minicv::EImageType::UINT8_GRAYSCALE, std::array<std::uint8_t, 3>{ 0, 127, 255 });
			minicv::Image resized = minicv::CreateResizedImage(image, image.GetSize(), interpolation);
			assert(resized.HasSameContent(image));
			resized.Fill(42);
			AssertPixelsEqual(image, std::array<std::uint8_t, 3>{ 0, 127, 255 });

			minicv::Image singlePixel(1, 1, minicv::EImageType::UINT8_RGB);
			singlePixel.FillRgb(0, 127, 255);
			const minicv::Image enlarged = minicv::CreateResizedImage(singlePixel, minicv::Size{ 3, 2 }, interpolation);
			singlePixel.Fill(0);
			AssertPixelsEqual(enlarged, std::array<std::uint8_t, 18>
			{
				0, 127, 255, 0, 127, 255, 0, 127, 255,
				0, 127, 255, 0, 127, 255, 0, 127, 255
			});
		}
	}

	void TestEmptyShapesPreserveTypesAndRequestedDimensions()
	{
		const std::array<minicv::Size, 3> sizes{ minicv::Size{ 0, 0 }, minicv::Size{ 0, 5 }, minicv::Size{ std::numeric_limits<int>::max() / 3, 0 } };
		const std::array<minicv::EImageType, 2> types{ minicv::EImageType::UINT8_GRAYSCALE, minicv::EImageType::UINT8_RGB };
		for (const minicv::EImageType type : types)
		{
			for (const minicv::Size size : sizes)
			{
				const minicv::Image image(size, type);
				assert(minicv::CreateHorizontallyFlippedImage(image).HasSameShape(image));
				assert(minicv::CreateVerticallyFlippedImage(image).HasSameShape(image));
				const minicv::Image clockwise = minicv::CreateRotated90Image(image, minicv::ERotationDirection::CLOCKWISE);
				const minicv::Image counterclockwise = minicv::CreateRotated90Image(image, minicv::ERotationDirection::COUNTERCLOCKWISE);
				assert(clockwise.IsEmpty() && clockwise.GetImageType() == type);
				assert(clockwise.GetWidth() == size.Height && clockwise.GetHeight() == size.Width);
				assert(counterclockwise.HasSameShape(clockwise));

				const minicv::Image nearest = minicv::CreateResizedImage(image, minicv::Size{ 0, 7 }, minicv::EInterpolationType::NEAREST_NEIGHBOR);
				const minicv::Image bilinear = minicv::CreateResizedImage(image, minicv::Size{ 7, 0 }, minicv::EInterpolationType::BILINEAR);
				assert(nearest.IsEmpty() && nearest.GetImageType() == type);
				assert(nearest.GetWidth() == 0 && nearest.GetHeight() == 7);
				assert(bilinear.IsEmpty() && bilinear.GetImageType() == type);
				assert(bilinear.GetWidth() == 7 && bilinear.GetHeight() == 0);
			}
		}

		const minicv::Image image(2, 3);
		assert(minicv::CreateResizedImage(image, minicv::Size{ 0, 4 }, minicv::EInterpolationType::BILINEAR).IsEmpty());
	}

	void TestReversibleTransformsPreserveContentAndSource()
	{
		for (int height = 1; height <= 4; ++height)
		{
			for (int width = 1; width <= 4; ++width)
			{
				minicv::Image image(width, height, minicv::EImageType::UINT8_RGB);
				for (std::size_t index = 0; index < image.GetByteCount(); ++index)
				{
					image.GetPixelData()[index] = static_cast<std::uint8_t>(index);
				}
				const minicv::Image original = image.Clone();
				const minicv::Image horizontal = minicv::CreateHorizontallyFlippedImage(image);
				const minicv::Image vertical = minicv::CreateVerticallyFlippedImage(image);
				const minicv::Image clockwise = minicv::CreateRotated90Image(image, minicv::ERotationDirection::CLOCKWISE);
				assert(minicv::CreateHorizontallyFlippedImage(horizontal).HasSameContent(original));
				assert(minicv::CreateVerticallyFlippedImage(vertical).HasSameContent(original));
				assert(minicv::CreateRotated90Image(clockwise, minicv::ERotationDirection::COUNTERCLOCKWISE).HasSameContent(original));
				assert(image.HasSameContent(original));
				image.Fill(255);
				assert(minicv::CreateHorizontallyFlippedImage(horizontal).HasSameContent(original));
				assert(minicv::CreateVerticallyFlippedImage(vertical).HasSameContent(original));
				assert(minicv::CreateRotated90Image(clockwise, minicv::ERotationDirection::COUNTERCLOCKWISE).HasSameContent(original));
			}
		}
	}
}

void RunImageGeometryTests()
{
	TestNearestResizeUsesFloorCoordinates();
	TestBilinearResizeUsesPixelCentersAndClampedEdges();
	TestBilinearResizeHandlesSingleRowsAndColumns();
	TestBilinearResizeRoundsHalfUp();
	TestResizePreservesRgbChannelOrder();
	TestFlipsAndRotationsUseImageCoordinates();
	TestRgbFlipsAndRotationsCopyWholePixels();
	TestIdentityResizeAndSinglePixelExpansionOwnTheirPixels();
	TestEmptyShapesPreserveTypesAndRequestedDimensions();
	TestReversibleTransformsPreserveContentAndSource();
}
