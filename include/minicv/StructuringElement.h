#pragma once

#include <cstdint>
#include <vector>

#include "minicv/Size.h"

namespace minicv
{
	class StructuringElement
	{
	public:
		/**
		 * Requires positive, odd dimensions and width * height row-major mask values.
		 * Zero excludes a position; nonzero includes it. At least one position must be active.
		 * The anchor is fixed at (width / 2, height / 2).
		 */
		StructuringElement(const Size size, std::vector<std::uint8_t> maskValues);

		[[nodiscard]] int GetWidth() const;
		[[nodiscard]] int GetHeight() const;
		[[nodiscard]] Size GetSize() const;

		/** Requires coordinates inside the structuring element. */
		[[nodiscard]] bool IsActive(const int x, const int y) const;

	private:
		int mWidth;
		int mHeight;
		std::vector<std::uint8_t> mMaskValues;
	};
}
