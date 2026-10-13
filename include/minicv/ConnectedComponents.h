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
		// Requires UINT8_GRAYSCALE input and FOUR or EIGHT connectivity (checked by assert).
		// Nonzero pixels are foreground; background label 0 is excluded from counts and statistics.
		// Labels start at 1, assigned by first occurrence in row-major order.
		// Results own their data; empty input preserves its size and has no components.
		ConnectedComponents(const Image& grayscaleImage, const EConnectivity connectivity);

		ConnectedComponents(const ConnectedComponents& other) = default;

		// Copy assignment preserves the current value if allocation fails.
		ConnectedComponents& operator=(const ConnectedComponents& other);

		// Moving leaves the source empty with size 0 x 0; self-move preserves its value.
		ConnectedComponents(ConnectedComponents&& other) noexcept;
		ConnectedComponents& operator=(ConnectedComponents&& other) noexcept;

		// Reports an empty pixel grid, not the absence of foreground components.
		[[nodiscard]] bool IsEmpty() const;
		[[nodiscard]] int GetWidth() const;
		[[nodiscard]] int GetHeight() const;
		[[nodiscard]] Size GetSize() const;

		[[nodiscard]] std::size_t GetLabel(const int x, const int y) const;

		[[nodiscard]] std::size_t GetComponentCount() const;

		// Statistics for label n are at index n - 1.
		// The reference is valid until this object is destroyed, assigned to, or moved from.
		[[nodiscard]] const std::vector<ConnectedComponent>& GetComponents() const;

	private:
		int mWidth;
		int mHeight;
		std::vector<std::size_t> mLabels;
		std::vector<ConnectedComponent> mComponents;
	};
}
