#pragma once

#include <cstdint>

#include "minicv/EBorderType.h"

namespace minicv
{
	struct ImageBorderParameters
	{
		EBorderType BorderType;
		std::uint8_t ConstantBorderValue;
	};
}
