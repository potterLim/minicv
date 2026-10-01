#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

#include "StructuringElementTest.h"
#include "minicv/ImageMorphology.h"
#include "minicv/StructuringElement.h"

namespace
{
	void TestMoveSemantics()
	{
		minicv::StructuringElement source(minicv::Size{ 3, 1 }, { 0, 1, 0 });
		minicv::StructuringElement moved(std::move(source));
		assert(moved.GetWidth() == 3 && moved.GetHeight() == 1);
		assert(moved.IsActive(1, 0));
		assert(moved.IsActive(0, 0) == false);
		assert(source.GetWidth() == 1 && source.GetHeight() == 1);
		assert(source.IsActive(0, 0));

		minicv::StructuringElement destination(minicv::Size{ 1, 1 }, { 1 });
		destination = std::move(moved);
		assert(destination.GetWidth() == 3 && destination.GetHeight() == 1);
		assert(destination.IsActive(1, 0));
		assert(destination.IsActive(0, 0) == false);
		assert(moved.GetWidth() == 1 && moved.GetHeight() == 1);
		assert(moved.IsActive(0, 0));

		minicv::StructuringElement* const alias = &destination;
		destination = std::move(*alias);
		assert(destination.GetWidth() == 3 && destination.GetHeight() == 1);
		assert(destination.IsActive(1, 0));
		assert(destination.IsActive(0, 0) == false);

		const minicv::StructuringElement copied(destination);
		source = copied;
		assert(copied.GetWidth() == 3 && copied.GetHeight() == 1);
		assert(copied.IsActive(1, 0));
		assert(copied.IsActive(0, 0) == false);
		assert(source.GetWidth() == 3 && source.GetHeight() == 1);
		assert(source.IsActive(1, 0));
		assert(source.IsActive(0, 0) == false);
	}

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
	TestMoveSemantics();
	TestStructuringElementStoresSizeAndRowMajorMask();
	TestRectangularStructuringElementActivatesEveryPosition();
	TestCrossStructuringElementActivatesOnlyCentralAxes();
	TestSinglePositionStructuringElementsAreActive();
}
