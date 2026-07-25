#pragma once

#include <array>

#include "minicv/GrayscaleHistogram.h"

namespace minicv
{
	struct GrayscaleCumulativeDistribution
	{
		std::array<double, GRAYSCALE_HISTOGRAM_BIN_COUNT> Values;
	};
}
