#pragma once

#include <array>
#include <cstddef>

namespace minicv
{
	inline constexpr std::size_t GRAYSCALE_HISTOGRAM_BIN_COUNT = 256;

	struct GrayscaleHistogram
	{
		std::array<std::size_t, GRAYSCALE_HISTOGRAM_BIN_COUNT> BinCounts;
	};
}
