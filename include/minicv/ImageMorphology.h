#pragma once

#include "minicv/Image.h"
#include "minicv/ImageBorderParameters.h"
#include "minicv/Size.h"
#include "minicv/StructuringElement.h"

namespace minicv
{
	// Requires positive, odd dimensions.
	[[nodiscard]] StructuringElement CreateRectangularStructuringElement(const Size size);

	// Requires positive, odd dimensions; the cross spans the full center row and column.
	[[nodiscard]] StructuringElement CreateCrossStructuringElement(const Size size);

	// RGB channels are processed independently.
	// Results own their pixels and preserve size and type, including empty shapes.
	// CONSTANT applies ConstantBorderValue to intermediate images as well.
	// Offsets are relative to the structuring element's center.

	// Takes the minimum over active offsets: source(x + dx, y + dy).
	[[nodiscard]] Image CreateErodedImage(
		const Image& image,
		const StructuringElement& structuringElement,
		const ImageBorderParameters borderParameters);

	// Takes the maximum over reflected active offsets: source(x - dx, y - dy).
	[[nodiscard]] Image CreateDilatedImage(
		const Image& image,
		const StructuringElement& structuringElement,
		const ImageBorderParameters borderParameters);

	[[nodiscard]] Image CreateOpenedImage(
		const Image& image,
		const StructuringElement& structuringElement,
		const ImageBorderParameters borderParameters);

	[[nodiscard]] Image CreateClosedImage(
		const Image& image,
		const StructuringElement& structuringElement,
		const ImageBorderParameters borderParameters);
}
