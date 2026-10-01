#pragma once

#include <vector>

#include "minicv/Size.h"

namespace minicv
{
	class GrayscaleFilterResponse
	{
	public:
		explicit GrayscaleFilterResponse(const Size size);

		GrayscaleFilterResponse(const GrayscaleFilterResponse& other) = default;
		/** Copy assignment preserves the current value if allocation fails. */
		GrayscaleFilterResponse& operator=(const GrayscaleFilterResponse& other);

		/** Moving leaves the source empty with size 0 x 0; self-move preserves its value. */
		GrayscaleFilterResponse(GrayscaleFilterResponse&& other) noexcept;
		GrayscaleFilterResponse& operator=(GrayscaleFilterResponse&& other) noexcept;

		[[nodiscard]] bool IsEmpty() const;
		[[nodiscard]] int GetWidth() const;
		[[nodiscard]] int GetHeight() const;
		[[nodiscard]] Size GetSize() const;

		[[nodiscard]] double& GetResponseValue(const int x, const int y);
		[[nodiscard]] const double& GetResponseValue(const int x, const int y) const;

	private:
		int mWidth;
		int mHeight;
		std::vector<double> mResponseValues;
	};
}
