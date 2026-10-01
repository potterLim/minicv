#pragma once

#include <cstddef>

#include "minicv/Rect.h"

namespace minicv
{
	/** Statistics for one foreground component; background label 0 is excluded. */
	struct ConnectedComponent
	{
		std::size_t Label;
		std::size_t Area;
		Rect BoundingBox;
		double CentroidX;
		double CentroidY;
	};
}
