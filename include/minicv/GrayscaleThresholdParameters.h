#pragma once

#include <cstdint>

#include "minicv/EThresholdType.h"

namespace minicv
{
	struct GrayscaleThresholdParameters
	{
		EThresholdType ThresholdType;
		std::uint8_t ThresholdValue;
		std::uint8_t MaximumValue;
	};
}
