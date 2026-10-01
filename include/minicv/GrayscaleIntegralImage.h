#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "minicv/Rect.h"
#include "minicv/Size.h"

namespace minicv
{
	class Image;

	class GrayscaleIntegralImage
	{
	public:
		explicit GrayscaleIntegralImage(const Image& grayscaleImage);

		GrayscaleIntegralImage(const GrayscaleIntegralImage& other) = default;
		/** Copy assignment preserves the current value if allocation fails. */
		GrayscaleIntegralImage& operator=(const GrayscaleIntegralImage& other);

		/** Moving leaves the source empty with size 0 x 0; self-move preserves its value. */
		GrayscaleIntegralImage(GrayscaleIntegralImage&& other) noexcept;
		GrayscaleIntegralImage& operator=(GrayscaleIntegralImage&& other) noexcept;

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
