#include <cassert>
#include <cstdint>
#include <vector>

#include "StructuringElementTest.h"
#include "minicv/ImageMorphology.h"
#include "minicv/StructuringElement.h"

namespace
{
	void TestStructuringElementStoresSizeAndRowMajorMask()
	{
		std::vector<std::uint8_t> maskValues{ 0, 2, 0, 255, 0, 0, 0, 0, 1 };
		const minicv::StructuringElement element(minicv::Size{ 3, 3 }, maskValues);
		maskValues[1] = 0;
		const minicv::Size size = element.GetSize();

		assert(element.GetWidth() == 3 && element.GetHeight() == 3);
		assert(size.Width == 3 && size.Height == 3);
		assert(element.IsActive(1, 0));
		assert(element.IsActive(0, 1));
		assert(element.IsActive(2, 2));
		assert(!element.IsActive(0, 0));
		assert(!element.IsActive(1, 1));
		assert(!element.IsActive(2, 0));
		static_cast<void>(size);
	}

	void TestRectangularStructuringElementActivatesEveryPosition()
	{
		const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 5, 3 });
		assert(element.GetWidth() == 5 && element.GetHeight() == 3);
		for (int y = 0; y < element.GetHeight(); ++y)
		{
			for (int x = 0; x < element.GetWidth(); ++x)
			{
				assert(element.IsActive(x, y));
			}
		}
	}

	void TestCrossStructuringElementActivatesOnlyCentralAxes()
	{
		const minicv::StructuringElement element = minicv::CreateCrossStructuringElement(minicv::Size{ 5, 3 });
		assert(element.GetWidth() == 5 && element.GetHeight() == 3);
		for (int y = 0; y < element.GetHeight(); ++y)
		{
			for (int x = 0; x < element.GetWidth(); ++x)
			{
				const bool isExpectedActive = x == 2 || y == 1;
				assert(element.IsActive(x, y) == isExpectedActive);
				static_cast<void>(isExpectedActive);
			}
		}
	}

	void TestSinglePositionStructuringElementsAreActive()
	{
		const minicv::StructuringElement rectangle = minicv::CreateRectangularStructuringElement(minicv::Size{ 1, 1 });
		const minicv::StructuringElement cross = minicv::CreateCrossStructuringElement(minicv::Size{ 1, 1 });
		assert(rectangle.IsActive(0, 0));
		assert(cross.IsActive(0, 0));
	}
}

void RunStructuringElementTests()
{
	TestStructuringElementStoresSizeAndRowMajorMask();
	TestRectangularStructuringElementActivatesEveryPosition();
	TestCrossStructuringElementActivatesOnlyCentralAxes();
	TestSinglePositionStructuringElementsAreActive();
}
