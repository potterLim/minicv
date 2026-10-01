#pragma once

#include <vector>

#include "minicv/Size.h"

namespace minicv
{
	class ConvolutionKernel
	{
	public:
		ConvolutionKernel(const Size size, std::vector<double> coefficients);

		ConvolutionKernel(const ConvolutionKernel& other) = default;
		/** Copy assignment preserves the current value if allocation fails. */
		ConvolutionKernel& operator=(const ConvolutionKernel& other);

		/** Move construction leaves a 1 x 1 identity; move assignment swaps valid values. */
		ConvolutionKernel(ConvolutionKernel&& other);
		ConvolutionKernel& operator=(ConvolutionKernel&& other) noexcept;

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
