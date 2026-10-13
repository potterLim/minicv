#pragma once

#include "minicv/EInterpolationType.h"
#include "minicv/ERotationDirection.h"
#include "minicv/Image.h"
#include "minicv/Size.h"

namespace minicv
{
	// Results own their pixels and preserve image type and channel order.
	// Result sizes must be representable by Image.
	// Invalid sizes or enum values are precondition violations checked by assert.

	// Target dimensions must be nonnegative.
	// A zero dimension produces an empty image with the exact target size.
	// A nonempty target requires a nonempty source.
	// NEAREST_NEIGHBOR uses floor(position * sourceLength / targetLength) on each axis.
	// BILINEAR uses (position + 0.5) * sourceLength / targetLength - 0.5 on each axis.
	// Bilinear source coordinates are clamped to the image edges.
	// Bilinear results round to the nearest integer (halfway upward) and clamp to [0, 255].
	[[nodiscard]] Image CreateResizedImage(const Image& image, const Size targetSize, const EInterpolationType interpolationType);

	// Both flips preserve size, including empty shapes.
	[[nodiscard]] Image CreateHorizontallyFlippedImage(const Image& image);

	[[nodiscard]] Image CreateVerticallyFlippedImage(const Image& image);

	// Swaps width and height, including for empty input.
	// Directions use image coordinates: X rightward, Y downward.
	[[nodiscard]] Image CreateRotated90Image(const Image& image, const ERotationDirection rotationDirection);
}
