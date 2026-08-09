#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "minicv/Image.h"
#include "minicv/Rect.h"
#include "minicv/Size.h"

namespace minicv
{
	class GrayscaleIntegralImage
	{
	public:
		explicit GrayscaleIntegralImage(const Image& grayscaleImage);

		[[nodiscard]] bool IsEmpty() const;
		[[nodiscard]] int GetWidth() const;
		[[nodiscard]] int GetHeight() const;
		[[nodiscard]] Size GetSize() const;
		[[nodiscard]] std::uint64_t GetRegionSum(const Rect region) const;

	private:
		int mWidth;
		int mHeight;
		std::size_t mValuesPerRow;
		std::vector<std::uint64_t> mIntegralValues;
	};
}
