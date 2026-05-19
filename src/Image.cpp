#include <cassert>
#include <cstddef>
#include <limits>

#include "minicv/Image.h"

namespace minicv
{
	namespace
	{
		int GetChannelCountFromImageType(const EImageType imageType)
		{
			switch (imageType)
			{
			case EImageType::UInt8Grayscale:
				return 1;
			case EImageType::UInt8RGB:
				return 3;
			default:
				assert(false && "Unsupported image type.");
				return 0;
			}
		}

		int CalculateBytesPerRow(const int width, const int channelCount)
		{
			assert(width >= 0 && "width must not be negative.");
			assert(channelCount > 0 && "channel count must be positive.");

			const std::size_t widthSize = static_cast<std::size_t>(width);
			const std::size_t channelCountSize = static_cast<std::size_t>(channelCount);
			const std::size_t maxBytesPerRow = static_cast<std::size_t>(std::numeric_limits<int>::max());
			const bool bytesPerRowFitsInInt = widthSize <= maxBytesPerRow / channelCountSize;

			assert(bytesPerRowFitsInInt && "bytes per row exceeds int range.");

			return static_cast<int>(widthSize * channelCountSize);
		}

		std::size_t CalculateByteCount(const int height, const int bytesPerRow)
		{
			assert(height >= 0 && "height must not be negative.");
			assert(bytesPerRow >= 0 && "bytes per row must not be negative.");

			const std::size_t heightSize = static_cast<std::size_t>(height);
			const std::size_t bytesPerRowSize = static_cast<std::size_t>(bytesPerRow);
			const bool byteCountDoesNotOverflow = bytesPerRowSize == 0 || heightSize <= std::numeric_limits<std::size_t>::max() / bytesPerRowSize;

			assert(byteCountDoesNotOverflow && "image byte count overflow.");

			return heightSize * bytesPerRowSize;
		}
	}

	Image::Image()
		: Image(0, 0, EImageType::UInt8Grayscale)
	{
	}

	Image::Image(const int width, const int height)
		: Image(width, height, EImageType::UInt8Grayscale)
	{
	}

	Image::Image(const int width, const int height, const EImageType imageType)
		: mWidth(width)
		, mHeight(height)
		, mChannelCount(GetChannelCountFromImageType(imageType))
		, mBytesPerRow(CalculateBytesPerRow(mWidth, mChannelCount))
		, mImageType(imageType)
	{
		const std::size_t byteCount = CalculateByteCount(mHeight, mBytesPerRow);
		assert(byteCount <= mPixels.max_size() && "image byte count exceeds maximum vector size.");

		mPixels.resize(byteCount);
	}

	bool Image::IsEmpty() const
	{
		return mPixels.empty();
	}

	int Image::GetWidth() const
	{
		return mWidth;
	}

	int Image::GetHeight() const
	{
		return mHeight;
	}

	int Image::GetChannelCount() const
	{
		return mChannelCount;
	}

	int Image::GetBytesPerRow() const
	{
		return mBytesPerRow;
	}

	EImageType Image::GetImageType() const
	{
		return mImageType;
	}

	std::size_t Image::GetPixelCount() const
	{
		return static_cast<std::size_t>(mWidth) * static_cast<std::size_t>(mHeight);
	}

	std::size_t Image::GetByteCount() const
	{
		return static_cast<std::size_t>(mHeight) * static_cast<std::size_t>(mBytesPerRow);
	}

	std::uint8_t* Image::GetPixelData()
	{
		return mPixels.data();
	}

	const std::uint8_t* Image::GetPixelData() const
	{
		return mPixels.data();
	}

	std::uint8_t& Image::GetGrayscalePixel(const int x, const int y)
	{
		assert(mImageType == EImageType::UInt8Grayscale && "image type must be UInt8Grayscale.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow) + static_cast<std::size_t>(x);

		return mPixels[index];
	}

	const std::uint8_t& Image::GetGrayscalePixel(const int x, const int y) const
	{
		assert(mImageType == EImageType::UInt8Grayscale && "image type must be UInt8Grayscale.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow) + static_cast<std::size_t>(x);

		return mPixels[index];
	}

	std::uint8_t& Image::GetRgbPixel(const int x, const int y, const ERgbChannel rgbChannel)
	{
		assert(mImageType == EImageType::UInt8RGB && "image type must be UInt8RGB.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t channelIndex = static_cast<std::size_t>(rgbChannel);
		assert(channelIndex < static_cast<std::size_t>(mChannelCount) && "rgbChannel is out of range.");

		const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow) + static_cast<std::size_t>(x) * static_cast<std::size_t>(mChannelCount) + channelIndex;

		return mPixels[index];
	}

	const std::uint8_t& Image::GetRgbPixel(const int x, const int y, const ERgbChannel rgbChannel) const
	{
		assert(mImageType == EImageType::UInt8RGB && "image type must be UInt8RGB.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t channelIndex = static_cast<std::size_t>(rgbChannel);
		assert(channelIndex < static_cast<std::size_t>(mChannelCount) && "rgbChannel is out of range.");

		const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow) + static_cast<std::size_t>(x) * static_cast<std::size_t>(mChannelCount) + channelIndex;

		return mPixels[index];
	}
}
