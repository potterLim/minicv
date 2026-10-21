#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "ObjectCandidateDetectionTest.h"
#include "minicv/ConnectedComponents.h"
#include "minicv/ImageMorphology.h"
#include "minicv/ImageOperations.h"
#include "minicv/Rect.h"

namespace
{
	constexpr minicv::GrayscaleThresholdParameters THRESHOLD_PARAMETERS{ minicv::EThresholdType::BINARY, 100, 255 };
	constexpr minicv::ImageBorderParameters BORDER_PARAMETERS{ minicv::EBorderType::REPLICATE, 0 };

	void FillRegion(minicv::Image* outImage, const minicv::Rect region, const std::uint8_t value)
	{
		assert(outImage != nullptr);
		for (int y = region.Y; y < region.Y + region.Height; ++y)
		{
			for (int x = region.X; x < region.X + region.Width; ++x)
			{
				outImage->GetGrayscalePixel(x, y) = value;
			}
		}
	}

	void AssertComponentMatchesRectangle(const minicv::ConnectedComponent& component, const minicv::Rect expectedRegion)
	{
		assert(component.Area == static_cast<std::size_t>(expectedRegion.Width) * expectedRegion.Height);
		assert(component.BoundingBox.X == expectedRegion.X && component.BoundingBox.Y == expectedRegion.Y);
		assert(component.BoundingBox.Width == expectedRegion.Width && component.BoundingBox.Height == expectedRegion.Height);
		assert(component.CentroidX == expectedRegion.X + (expectedRegion.Width - 1) / 2.0);
		assert(component.CentroidY == expectedRegion.Y + (expectedRegion.Height - 1) / 2.0);
	}

	void TestNoiseRemovalAndRegionStatistics()
	{
		minicv::Image source(24, 18);
		source.Fill(30);
		constexpr minicv::Rect OBJECT_REGION{ 3, 3, 9, 9 };
		constexpr minicv::Rect SMALL_REGION{ 17, 5, 3, 3 };
		FillRegion(&source, OBJECT_REGION, 180);
		FillRegion(&source, SMALL_REGION, 180);
		source.GetGrayscalePixel(7, 7) = 30;
		source.GetGrayscalePixel(20, 14) = 180;
		const minicv::Image original = source;

		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 3 });
		const minicv::Image binary = minicv::CreateThresholdedGrayscaleImage(source, THRESHOLD_PARAMETERS);
		const minicv::Image opened = minicv::CreateOpenedImage(binary, element, BORDER_PARAMETERS);
		const minicv::Image cleaned = minicv::CreateClosedImage(opened, element, BORDER_PARAMETERS);
		const minicv::ConnectedComponents components(cleaned, minicv::EConnectivity::EIGHT);

		assert(binary.GetGrayscalePixel(20, 14) == 255);
		assert(opened.GetGrayscalePixel(20, 14) == 0);
		assert(opened.GetGrayscalePixel(7, 7) == 0);
		assert(cleaned.GetGrayscalePixel(7, 7) == 255);

		minicv::Image expected(24, 18);
		expected.Fill(0);
		FillRegion(&expected, OBJECT_REGION, 255);
		FillRegion(&expected, SMALL_REGION, 255);

		assert(cleaned.HasSameContent(expected));
		assert(source.HasSameContent(original));
		assert(components.GetComponentCount() == 2);
		AssertComponentMatchesRectangle(components.GetComponents()[0], OBJECT_REGION);
		AssertComponentMatchesRectangle(components.GetComponents()[1], SMALL_REGION);
		assert(components.GetLabel(3, 3) == 1);
		assert(components.GetLabel(17, 5) == 2);
		assert(components.GetLabel(20, 14) == 0);

		constexpr std::size_t MINIMUM_AREA = 20;
		std::size_t candidateCount = 0;
		for (const minicv::ConnectedComponent& component : components.GetComponents())
		{
			if (component.Area >= MINIMUM_AREA)
			{
				assert(component.Label == 1);
				++candidateCount;
			}
		}
		assert(candidateCount == 1);
	}

	void TestBoundaryObjectIsPreserved()
	{
		minicv::Image source(12, 10);
		source.Fill(30);
		constexpr minicv::Rect OBJECT_REGION{ 0, 0, 5, 5 };
		FillRegion(&source, OBJECT_REGION, 180);
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 3 });
		const minicv::Image binary = minicv::CreateThresholdedGrayscaleImage(source, THRESHOLD_PARAMETERS);
		const minicv::Image opened = minicv::CreateOpenedImage(binary, element, BORDER_PARAMETERS);
		const minicv::Image cleaned = minicv::CreateClosedImage(opened, element, BORDER_PARAMETERS);
		const minicv::ConnectedComponents components(cleaned, minicv::EConnectivity::EIGHT);

		assert(cleaned.HasSameContent(binary));
		assert(components.GetComponentCount() == 1);
		AssertComponentMatchesRectangle(components.GetComponents()[0], OBJECT_REGION);
	}

	void TestConnectivityAfterOpening()
	{
		minicv::Image source(10, 10);
		source.Fill(0);
		FillRegion(&source, minicv::Rect{ 2, 2, 3, 3 }, 180);
		FillRegion(&source, minicv::Rect{ 5, 5, 3, 3 }, 180);
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 3 });
		const minicv::Image binary = minicv::CreateThresholdedGrayscaleImage(source, THRESHOLD_PARAMETERS);
		const minicv::Image opened = minicv::CreateOpenedImage(binary, element, BORDER_PARAMETERS);
		const minicv::ConnectedComponents fourConnected(opened, minicv::EConnectivity::FOUR);
		const minicv::ConnectedComponents eightConnected(opened, minicv::EConnectivity::EIGHT);

		assert(opened.HasSameContent(binary));
		assert(fourConnected.GetComponentCount() == 2);
		assert(eightConnected.GetComponentCount() == 1);
		assert(eightConnected.GetComponents()[0].Area == 18);
		assert(eightConnected.GetComponents()[0].CentroidX == 4.5);
		assert(eightConnected.GetComponents()[0].CentroidY == 4.5);
	}

	void TestUniformAndEmptyInputs()
	{
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 3 });
		constexpr std::array SIZES
		{
			minicv::Size{ 0, 0 },
			minicv::Size{ 0, 5 },
			minicv::Size{ 5, 0 },
			minicv::Size{ 1, 1 },
			minicv::Size{ 7, 5 }
		};

		for (const minicv::Size size : SIZES)
		{
			for (const std::uint8_t value : std::array<std::uint8_t, 3>{ 0, 100, 180 })
			{
				minicv::Image source(size);
				source.Fill(value);
				const minicv::Image binary = minicv::CreateThresholdedGrayscaleImage(source, THRESHOLD_PARAMETERS);
				const minicv::Image opened = minicv::CreateOpenedImage(binary, element, BORDER_PARAMETERS);
				const minicv::Image cleaned = minicv::CreateClosedImage(opened, element, BORDER_PARAMETERS);
				const minicv::ConnectedComponents components(cleaned, minicv::EConnectivity::EIGHT);

				assert(cleaned.HasSameContent(binary));
				assert(components.GetWidth() == size.Width && components.GetHeight() == size.Height);
				if (source.IsEmpty() || value <= THRESHOLD_PARAMETERS.ThresholdValue)
				{
					assert(components.GetComponentCount() == 0);
					assert(minicv::CountNonZeroPixels(cleaned) == 0);
				}
				else
				{
					assert(components.GetComponentCount() == 1);
					AssertComponentMatchesRectangle(components.GetComponents()[0], minicv::Rect{ 0, 0, size.Width, size.Height });
				}
			}
		}
	}

	void TestNoiseOnlyInputProducesNoCandidates()
	{
		minicv::Image source(7, 7);
		source.Fill(100);
		source.GetGrayscalePixel(3, 3) = 101;
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 3 });
		const minicv::Image binary = minicv::CreateThresholdedGrayscaleImage(source, THRESHOLD_PARAMETERS);
		const minicv::Image opened = minicv::CreateOpenedImage(binary, element, BORDER_PARAMETERS);
		const minicv::Image cleaned = minicv::CreateClosedImage(opened, element, BORDER_PARAMETERS);
		const minicv::ConnectedComponents components(cleaned, minicv::EConnectivity::EIGHT);

		assert(minicv::CountNonZeroPixels(binary) == 1);
		assert(minicv::CountNonZeroPixels(cleaned) == 0);
		assert(!components.IsEmpty());
		assert(components.GetComponentCount() == 0);
	}
}

void RunObjectCandidateDetectionTests()
{
	TestNoiseRemovalAndRegionStatistics();
	TestBoundaryObjectIsPreserved();
	TestConnectivityAfterOpening();
	TestUniformAndEmptyInputs();
	TestNoiseOnlyInputProducesNoCandidates();
}
