#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

#include "ConnectedComponentsTest.h"
#include "minicv/ConnectedComponents.h"
#include "minicv/Image.h"

namespace
{
	constexpr double COMPARISON_TOLERANCE = 1e-12;

	void AssertComponentEqual(const minicv::ConnectedComponent& actual, const minicv::ConnectedComponent& expected)
	{
		const bool isEqual = actual.Label == expected.Label && actual.Area == expected.Area &&
			actual.BoundingBox.X == expected.BoundingBox.X && actual.BoundingBox.Y == expected.BoundingBox.Y &&
			actual.BoundingBox.Width == expected.BoundingBox.Width && actual.BoundingBox.Height == expected.BoundingBox.Height &&
			std::abs(actual.CentroidX - expected.CentroidX) <= COMPARISON_TOLERANCE &&
			std::abs(actual.CentroidY - expected.CentroidY) <= COMPARISON_TOLERANCE;
		assert(isEqual);
		static_cast<void>(isEqual);
	}

	template <std::size_t PIXEL_COUNT>
	minicv::Image CreateImage(const minicv::Size size, const std::array<std::uint8_t, PIXEL_COUNT>& pixels)
	{
		minicv::Image image(size);
		assert(image.GetPixelCount() == PIXEL_COUNT);
		for (std::size_t index = 0; index < PIXEL_COUNT; ++index)
		{
			image.GetPixelData()[index] = pixels[index];
		}
		return image;
	}

	template <std::size_t LABEL_COUNT>
	void AssertLabelsEqual(const minicv::ConnectedComponents& components, const std::array<std::size_t, LABEL_COUNT>& labels)
	{
		assert(static_cast<std::size_t>(components.GetWidth()) * static_cast<std::size_t>(components.GetHeight()) == LABEL_COUNT);
		std::size_t index = 0;
		for (int y = 0; y < components.GetHeight(); ++y)
		{
			for (int x = 0; x < components.GetWidth(); ++x)
			{
				const bool isLabelEqual = components.GetLabel(x, y) == labels[index];
				assert(isLabelEqual);
				static_cast<void>(isLabelEqual);
				++index;
			}
		}
	}

	void TestEmptyShapesArePreserved()
	{
		const std::array<minicv::Size, 3> sizes{ minicv::Size{ 0, 0 }, minicv::Size{ 0, 4 }, minicv::Size{ 3, 0 } };
		for (const minicv::Size size : sizes)
		{
			for (const minicv::EConnectivity connectivity : { minicv::EConnectivity::FOUR, minicv::EConnectivity::EIGHT })
			{
				const minicv::ConnectedComponents components(minicv::Image(size), connectivity);
				const minicv::Size actualSize = components.GetSize();
				assert(components.IsEmpty());
				assert(actualSize.Width == size.Width && actualSize.Height == size.Height);
				assert(components.GetComponentCount() == 0 && components.GetComponents().empty());
				static_cast<void>(actualSize);
			}
		}
	}

	void TestBackgroundHasNoComponentsButIsNotEmpty()
	{
		minicv::Image image(3, 2);
		image.Fill(0);
		const minicv::ConnectedComponents components(image, minicv::EConnectivity::FOUR);
		assert(!components.IsEmpty());
		assert(components.GetComponentCount() == 0 && components.GetComponents().empty());
		AssertLabelsEqual(components, std::array<std::size_t, 6>{ 0, 0, 0, 0, 0, 0 });
	}

	void TestDiagonalConnectivityChangesComponentCount()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 2, 2 }, std::array<std::uint8_t, 4>{ 1, 0, 0, 255 });
		const minicv::ConnectedComponents four(image, minicv::EConnectivity::FOUR);
		const minicv::ConnectedComponents eight(image, minicv::EConnectivity::EIGHT);
		assert(four.GetComponentCount() == 2 && eight.GetComponentCount() == 1);
		AssertLabelsEqual(four, std::array<std::size_t, 4>{ 1, 0, 0, 2 });
		AssertLabelsEqual(eight, std::array<std::size_t, 4>{ 1, 0, 0, 1 });
		AssertComponentEqual(four.GetComponents()[0], minicv::ConnectedComponent{ 1, 1, { 0, 0, 1, 1 }, 0.0, 0.0 });
		AssertComponentEqual(four.GetComponents()[1], minicv::ConnectedComponent{ 2, 1, { 1, 1, 1, 1 }, 1.0, 1.0 });
		AssertComponentEqual(eight.GetComponents()[0], minicv::ConnectedComponent{ 1, 2, { 0, 0, 2, 2 }, 0.5, 0.5 });
	}

	void TestScanOrderAndPixelCentroidForIrregularRegion()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 5, 3 },
			std::array<std::uint8_t, 15>{ 0, 0, 0, 0, 255, 1, 2, 0, 0, 0, 3, 0, 0, 0, 0 });
		const minicv::ConnectedComponents components(image, minicv::EConnectivity::EIGHT);
		assert(components.GetComponentCount() == 2);
		AssertLabelsEqual(components, std::array<std::size_t, 15>{ 0, 0, 0, 0, 1, 2, 2, 0, 0, 0, 2, 0, 0, 0, 0 });
		AssertComponentEqual(components.GetComponents()[0], minicv::ConnectedComponent{ 1, 1, { 4, 0, 1, 1 }, 4.0, 0.0 });
		AssertComponentEqual(components.GetComponents()[1], minicv::ConnectedComponent{ 2, 3, { 0, 1, 2, 2 }, 1.0 / 3.0, 4.0 / 3.0 });
	}

	void TestLaterBridgeConnectsEarlierRuns()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 3, 2 }, std::array<std::uint8_t, 6>{ 10, 0, 20, 30, 40, 50 });
		const minicv::ConnectedComponents components(image, minicv::EConnectivity::FOUR);
		assert(components.GetComponentCount() == 1);
		AssertLabelsEqual(components, std::array<std::size_t, 6>{ 1, 0, 1, 1, 1, 1 });
		AssertComponentEqual(components.GetComponents()[0], minicv::ConnectedComponent{ 1, 5, { 0, 0, 3, 2 }, 1.0, 0.6 });
	}

	void TestHoleRemainsBackground()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 3, 3 }, std::array<std::uint8_t, 9>{ 1, 1, 1, 1, 0, 1, 1, 1, 1 });
		const minicv::ConnectedComponents components(image, minicv::EConnectivity::FOUR);
		assert(components.GetComponentCount() == 1);
		AssertLabelsEqual(components, std::array<std::size_t, 9>{ 1, 1, 1, 1, 0, 1, 1, 1, 1 });
		AssertComponentEqual(components.GetComponents()[0], minicv::ConnectedComponent{ 1, 8, { 0, 0, 3, 3 }, 1.0, 1.0 });
	}

	void TestLabelsExceedByteRange()
	{
		minicv::Image image(33, 33);
		image.Fill(0);
		for (int y = 0; y < image.GetHeight(); y += 2)
		{
			for (int x = 0; x < image.GetWidth(); x += 2)
			{
				image.GetGrayscalePixel(x, y) = 255;
			}
		}
		const minicv::ConnectedComponents components(image, minicv::EConnectivity::EIGHT);
		assert(components.GetComponentCount() == 289);
		std::size_t expectedLabel = 0;
		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				if (x % 2 == 0 && y % 2 == 0)
				{
					++expectedLabel;
					assert(components.GetLabel(x, y) == expectedLabel);
					AssertComponentEqual(components.GetComponents()[expectedLabel - 1],
						minicv::ConnectedComponent{ expectedLabel, 1, { x, y, 1, 1 }, static_cast<double>(x), static_cast<double>(y) });
				}
				else
				{
					assert(components.GetLabel(x, y) == 0);
				}
			}
		}
	}

	void TestLongSingleColumnUsesOwnedIterativeResults()
	{
		constexpr int HEIGHT = 100'000;
		minicv::Image image(1, HEIGHT);
		image.Fill(7);
		const minicv::ConnectedComponents components(image, minicv::EConnectivity::FOUR);
		assert(image.GetGrayscalePixel(0, 0) == 7 && image.GetGrayscalePixel(0, HEIGHT - 1) == 7);
		image.Fill(0);
		assert(components.GetComponentCount() == 1);
		assert(components.GetLabel(0, HEIGHT - 1) == 1);
		AssertComponentEqual(components.GetComponents()[0],
			minicv::ConnectedComponent{ 1, HEIGHT, { 0, 0, 1, HEIGHT }, 0.0, (HEIGHT - 1) / 2.0 });
	}

	void TestOppositeRowEdgesRemainSeparate()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 3, 2 }, std::array<std::uint8_t, 6>{ 0, 0, 1, 1, 0, 0 });
		const minicv::ConnectedComponents components(image, minicv::EConnectivity::EIGHT);
		assert(components.GetComponentCount() == 2);
		AssertLabelsEqual(components, std::array<std::size_t, 6>{ 0, 0, 1, 2, 0, 0 });
		AssertComponentEqual(components.GetComponents()[0], minicv::ConnectedComponent{ 1, 1, { 2, 0, 1, 1 }, 2.0, 0.0 });
		AssertComponentEqual(components.GetComponents()[1], minicv::ConnectedComponent{ 2, 1, { 0, 1, 1, 1 }, 0.0, 1.0 });
	}

	void TestSingleRowDoesNotConnectAcrossBackground()
	{
		const minicv::Image image = CreateImage(minicv::Size{ 5, 1 }, std::array<std::uint8_t, 5>{ 1, 2, 0, 3, 4 });
		const minicv::ConnectedComponents components(image, minicv::EConnectivity::EIGHT);
		assert(components.GetComponentCount() == 2);
		AssertLabelsEqual(components, std::array<std::size_t, 5>{ 1, 1, 0, 2, 2 });
		AssertComponentEqual(components.GetComponents()[0], minicv::ConnectedComponent{ 1, 2, { 0, 0, 2, 1 }, 0.5, 0.0 });
		AssertComponentEqual(components.GetComponents()[1], minicv::ConnectedComponent{ 2, 2, { 3, 0, 2, 1 }, 3.5, 0.0 });
	}
}

void RunConnectedComponentsTests()
{
	TestEmptyShapesArePreserved();
	TestBackgroundHasNoComponentsButIsNotEmpty();
	TestDiagonalConnectivityChangesComponentCount();
	TestScanOrderAndPixelCentroidForIrregularRegion();
	TestLaterBridgeConnectsEarlierRuns();
	TestHoleRemainsBackground();
	TestLabelsExceedByteRange();
	TestLongSingleColumnUsesOwnedIterativeResults();
	TestSingleRowDoesNotConnectAcrossBackground();
	TestOppositeRowEdgesRemainSeparate();
}
