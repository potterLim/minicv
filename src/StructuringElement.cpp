#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

#include "minicv/ImageMorphology.h"
#include "minicv/StructuringElement.h"

namespace minicv
{
	namespace
	{
		std::size_t CalculateMaskValueCount(const Size size)
		{
			assert(size.Width > 0 && size.Height > 0 && "structuring element dimensions must be positive.");
			assert(size.Width % 2 == 1 && size.Height % 2 == 1 && "structuring element dimensions must be odd.");

			const std::size_t width = static_cast<std::size_t>(size.Width);
			const std::size_t height = static_cast<std::size_t>(size.Height);
			assert(width <= std::numeric_limits<std::size_t>::max() / height && "mask value count overflow.");
			return width * height;
		}

		std::vector<std::uint8_t> CreateMaskValues(const Size size, const std::uint8_t initialValue)
		{
			const std::size_t maskValueCount = CalculateMaskValueCount(size);
			std::vector<std::uint8_t> maskValues;
			assert(maskValueCount <= maskValues.max_size() && "mask value count exceeds maximum vector size.");
			maskValues.resize(maskValueCount, initialValue);
			return maskValues;
		}
	}

	StructuringElement::StructuringElement(const Size size, std::vector<std::uint8_t> maskValues)
		: mWidth(size.Width)
		, mHeight(size.Height)
		, mMaskValues(std::move(maskValues))
	{
		const std::size_t expectedMaskValueCount = CalculateMaskValueCount(size);
		assert(mMaskValues.size() == expectedMaskValueCount && "mask value count must match structuring element size.");
		static_cast<void>(expectedMaskValueCount);

		const bool hasActivePosition = std::any_of(mMaskValues.begin(), mMaskValues.end(), [](const std::uint8_t maskValue)
			{
				return maskValue != 0;
			});
		assert(hasActivePosition && "structuring element must have at least one active position.");
		static_cast<void>(hasActivePosition);
	}

	int StructuringElement::GetWidth() const
	{
		return mWidth;
	}

	int StructuringElement::GetHeight() const
	{
		return mHeight;
	}

	Size StructuringElement::GetSize() const
	{
		return Size{ mWidth, mHeight };
	}

	bool StructuringElement::IsActive(const int x, const int y) const
	{
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");
		const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(mWidth);
		const std::size_t maskIndex = rowOffset + static_cast<std::size_t>(x);
		return mMaskValues[maskIndex] != 0;
	}

	StructuringElement CreateRectangularStructuringElement(const Size size)
	{
		return StructuringElement(size, CreateMaskValues(size, 1));
	}

	StructuringElement CreateCrossStructuringElement(const Size size)
	{
		std::vector<std::uint8_t> maskValues = CreateMaskValues(size, 0);
		const int centerX = size.Width / 2;
		const int centerY = size.Height / 2;
		for (int y = 0; y < size.Height; ++y)
		{
			const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(size.Width);
			for (int x = 0; x < size.Width; ++x)
			{
				if (x == centerX || y == centerY)
				{
					const std::size_t maskIndex = rowOffset + static_cast<std::size_t>(x);
					maskValues[maskIndex] = 1;
				}
			}
		}
		return StructuringElement(size, std::move(maskValues));
	}
}
