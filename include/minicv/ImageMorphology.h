#pragma once

#include "minicv/Image.h"
#include "minicv/ImageBorderParameters.h"
#include "minicv/Size.h"
#include "minicv/StructuringElement.h"

namespace minicv
{
	/** Requires positive, odd dimensions; activates every position. */
	[[nodiscard]] StructuringElement CreateRectangularStructuringElement(const Size size);

	/** Requires positive, odd dimensions; activates the center row and center column. */
	[[nodiscard]] StructuringElement CreateCrossStructuringElement(const Size size);

	/**
	 * Morphology operates independently on each UINT8_GRAYSCALE or UINT8_RGB channel.
	 * All operations preserve size and type, leave the input unchanged, and return
	 * an empty image for an empty input. Borders must be CONSTANT or REPLICATE;
	 * CONSTANT uses ConstantBorderValue directly, including for intermediate images.
	 * Offsets (dx, dy) are measured from the structuring element's central anchor.
	 */

	/** Takes the minimum over active offsets: source(x + dx, y + dy). */
	[[nodiscard]] Image CreateErodedImage(
		const Image& image,
		const StructuringElement& structuringElement,
		const ImageBorderParameters borderParameters);

	/** Takes the maximum over reflected active offsets: source(x - dx, y - dy). */
	[[nodiscard]] Image CreateDilatedImage(
		const Image& image,
		const StructuringElement& structuringElement,
		const ImageBorderParameters borderParameters);

	/** Erodes, then dilates with the same structuring element and border parameters. */
	[[nodiscard]] Image CreateOpenedImage(
		const Image& image,
		const StructuringElement& structuringElement,
		const ImageBorderParameters borderParameters);

	/** Dilates, then erodes with the same structuring element and border parameters. */
	[[nodiscard]] Image CreateClosedImage(
		const Image& image,
		const StructuringElement& structuringElement,
		const ImageBorderParameters borderParameters);
}
