#pragma once

#include <vector>

#include "minicv/Size.h"

namespace minicv
{
	class GrayscaleFilterResponse
	{
	public:
		explicit GrayscaleFilterResponse(const Size size);

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
