#pragma once

#include <vector>

#include "minicv/Size.h"

namespace minicv
{
	class ConvolutionKernel
	{
	public:
		ConvolutionKernel(const Size size, std::vector<double> coefficients);

		[[nodiscard]] int GetWidth() const;
		[[nodiscard]] int GetHeight() const;
		[[nodiscard]] Size GetSize() const;
		[[nodiscard]] double GetCoefficient(const int x, const int y) const;

	private:
		int mWidth;
		int mHeight;
		std::vector<double> mCoefficients;
	};
}
