#pragma once

#include <cstddef>

#include "minicv/Rect.h"

namespace minicv
{
	// Area and BoundingBox extents are measured in pixels, with a top-left origin.
	// Centroids average zero-based pixel indices, without a half-pixel offset.
	// X increases rightward and Y downward.
	struct ConnectedComponent
	{
		std::size_t Label;
		std::size_t Area;
		Rect BoundingBox;
		double CentroidX;
		double CentroidY;
	};
}
