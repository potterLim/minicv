#pragma once

#include <cstddef>
#include <vector>

#include "minicv/ConnectedComponent.h"
#include "minicv/EConnectivity.h"
#include "minicv/Size.h"

namespace minicv
{
	class Image;

	class ConnectedComponents
	{
	public:
		/**
		 * Labels UINT8_GRAYSCALE input: zero is background and every nonzero value
		 * is foreground, regardless of intensity. FOUR connects edge neighbors;
		 * EIGHT also connects diagonal neighbors. Other connectivity values are invalid.
		 * Background has label 0. Foreground labels are contiguous from 1, ordered
		 * by each component's first pixel in a top-to-bottom, left-to-right scan.
		 * Owns the results without retaining or modifying the input image.
		 * Empty input preserves its size and produces no component statistics.
		 * Invalid input type or connectivity is a precondition violation checked by assert.
		 */
		ConnectedComponents(const Image& grayscaleImage, const EConnectivity connectivity);

		ConnectedComponents(const ConnectedComponents& other) = default;
		/** Copy assignment preserves the current value if allocation fails. */
		ConnectedComponents& operator=(const ConnectedComponents& other);

		/** Moving leaves the source empty with size 0 x 0; self-move preserves its value. */
		ConnectedComponents(ConnectedComponents&& other) noexcept;
		ConnectedComponents& operator=(ConnectedComponents&& other) noexcept;

		/** Reports an empty pixel grid, not the absence of foreground components. */
		[[nodiscard]] bool IsEmpty() const;
		[[nodiscard]] int GetWidth() const;
		[[nodiscard]] int GetHeight() const;
		[[nodiscard]] Size GetSize() const;

		/** Requires coordinates inside the image; returns 0 for background. */
		[[nodiscard]] std::size_t GetLabel(const int x, const int y) const;

		/** Counts foreground components only. */
		[[nodiscard]] std::size_t GetComponentCount() const;

		/**
		 * Returns foreground statistics in label order; label n is at index n - 1.
		 * Area is the pixel count. BoundingBox uses a top-left origin and pixel extents.
		 * CentroidX/Y are the arithmetic means of pixel indices (no half-pixel offset),
		 * with X increasing rightward and Y downward. An all-zero image has no components.
		 * The reference is valid until this object is destroyed, assigned to, or moved from.
		 */
		[[nodiscard]] const std::vector<ConnectedComponent>& GetComponents() const;

	private:
		int mWidth;
		int mHeight;
		std::vector<std::size_t> mLabels;
		std::vector<ConnectedComponent> mComponents;
	};
}
