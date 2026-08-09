#pragma once

#include <cstdint>

#include "minicv/EThresholdType.h"

namespace minicv
{
	struct GrayscaleAdaptiveThresholdParameters
	{
		EThresholdType ThresholdType;
		std::uint8_t MaximumValue;
		int BlockSize;
		double MeanOffset;
	};
}
