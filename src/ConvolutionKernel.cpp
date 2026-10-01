#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>

#include "minicv/ConvolutionKernel.h"

namespace minicv
{
	namespace
	{
		std::size_t CalculateCoefficientCount(const int width, const int height)
		{
			assert(width > 0 && "kernel width must be positive.");
			assert(height > 0 && "kernel height must be positive.");

			const std::size_t widthSize = static_cast<std::size_t>(width);
			const std::size_t heightSize = static_cast<std::size_t>(height);

			assert(widthSize <= std::numeric_limits<std::size_t>::max() / heightSize && "kernel coefficient count overflow.");

			return widthSize * heightSize;
		}
	}

	ConvolutionKernel::ConvolutionKernel(const Size size, std::vector<double> coefficients)
		: mWidth(size.Width)
		, mHeight(size.Height)
		, mCoefficients(std::move(coefficients))
	{
		assert(mWidth % 2 == 1 && "kernel width must be odd.");
		assert(mHeight % 2 == 1 && "kernel height must be odd.");

		const std::size_t expectedCoefficientCount = CalculateCoefficientCount(mWidth, mHeight);
		assert(mCoefficients.size() == expectedCoefficientCount && "kernel coefficient count must match kernel size.");

		static_cast<void>(expectedCoefficientCount);

		for (const double coefficient : mCoefficients)
		{
			const bool isCoefficientFinite = std::isfinite(coefficient);
			assert(isCoefficientFinite && "kernel coefficients must be finite.");

			static_cast<void>(isCoefficientFinite);
		}
	}

	int ConvolutionKernel::GetWidth() const
	{
		return mWidth;
	}

	int ConvolutionKernel::GetHeight() const
	{
		return mHeight;
	}

	Size ConvolutionKernel::GetSize() const
	{
		return Size{ mWidth, mHeight };
	}

	double ConvolutionKernel::GetCoefficient(const int x, const int y) const
	{
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(mWidth);
		const std::size_t coefficientIndex = rowOffset + static_cast<std::size_t>(x);

		return mCoefficients[coefficientIndex];
	}
}
