#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

#include "minicv/GrayscaleFilterResponse.h"

namespace minicv
{
	namespace
	{
		std::size_t CalculateResponseValueCount(const int width, const int height)
		{
			assert(width >= 0 && "width must not be negative.");
			assert(height >= 0 && "height must not be negative.");

			const std::size_t widthSize = static_cast<std::size_t>(width);
			const std::size_t heightSize = static_cast<std::size_t>(height);

			assert((heightSize == 0 || widthSize <= std::numeric_limits<std::size_t>::max() / heightSize) && "response value count overflow.");

			return widthSize * heightSize;
		}
	}

	GrayscaleFilterResponse::GrayscaleFilterResponse(const Size size)
		: mWidth(size.Width)
		, mHeight(size.Height)
	{
		const std::size_t responseValueCount = CalculateResponseValueCount(mWidth, mHeight);
		assert(responseValueCount <= mResponseValues.max_size() && "response value count exceeds maximum vector size.");

		mResponseValues.resize(responseValueCount);
	}

	GrayscaleFilterResponse::GrayscaleFilterResponse(GrayscaleFilterResponse&& other) noexcept
		: mWidth(std::exchange(other.mWidth, 0))
		, mHeight(std::exchange(other.mHeight, 0))
		, mResponseValues(std::move(other.mResponseValues))
	{
		other.mResponseValues.clear();
	}

	GrayscaleFilterResponse& GrayscaleFilterResponse::operator=(const GrayscaleFilterResponse& other)
	{
		if (this == &other)
		{
			return *this;
		}

		GrayscaleFilterResponse copied(other);
		return *this = std::move(copied);
	}

	GrayscaleFilterResponse& GrayscaleFilterResponse::operator=(GrayscaleFilterResponse&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		mWidth = std::exchange(other.mWidth, 0);
		mHeight = std::exchange(other.mHeight, 0);
		mResponseValues = std::move(other.mResponseValues);
		other.mResponseValues.clear();
		return *this;
	}

	bool GrayscaleFilterResponse::IsEmpty() const
	{
		return mResponseValues.empty();
	}

	int GrayscaleFilterResponse::GetWidth() const
	{
		return mWidth;
	}

	int GrayscaleFilterResponse::GetHeight() const
	{
		return mHeight;
	}

	Size GrayscaleFilterResponse::GetSize() const
	{
		return Size{ mWidth, mHeight };
	}

	double& GrayscaleFilterResponse::GetResponseValue(const int x, const int y)
	{
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(mWidth);
		const std::size_t responseValueIndex = rowOffset + static_cast<std::size_t>(x);

		return mResponseValues[responseValueIndex];
	}

	const double& GrayscaleFilterResponse::GetResponseValue(const int x, const int y) const
	{
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(mWidth);
		const std::size_t responseValueIndex = rowOffset + static_cast<std::size_t>(x);

		return mResponseValues[responseValueIndex];
	}
}
