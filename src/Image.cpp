#include <cassert>

#include "minicv/Image.h"

namespace minicv
{
	namespace
	{
		int GetChannelCountFromImageType(const eImageType imageType)
		{
			switch (imageType)
			{
			case eImageType::UInt8Grayscale:
				return 1;
			case eImageType::UInt8RGB:
				return 3;
			default:
				assert(false && "Unsupported image type.");
				return 0;
			}
		}
	}

	Image::Image()
		: Image(0, 0, eImageType::UInt8Grayscale)
	{
	}

	Image::Image(const int width, const int height)
		: Image(width, height, eImageType::UInt8Grayscale)
	{
	}

	Image::Image(const int width, const int height, const eImageType imageType)
		: mWidth(width)
		, mHeight(height)
		, mChannelCount(GetChannelCountFromImageType(imageType))
		, mBytesPerRow(mWidth * mChannelCount)
		, mImageType(imageType)
	{
		assert(mWidth >= 0 && "width must not be negative.");
		assert(mHeight >= 0 && "height must not be negative.");

		mPixels.resize(static_cast<std::size_t>(mHeight) * static_cast<std::size_t>(mBytesPerRow));
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

	eImageType Image::GetImageType() const
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
		assert(mImageType == eImageType::UInt8Grayscale && "image type must be UInt8Grayscale.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow) + static_cast<std::size_t>(x);

		return mPixels[index];
	}

	const std::uint8_t& Image::GetGrayscalePixel(const int x, const int y) const
	{
		assert(mImageType == eImageType::UInt8Grayscale && "image type must be UInt8Grayscale.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow) + static_cast<std::size_t>(x);

		return mPixels[index];
	}

	std::uint8_t& Image::GetRgbPixel(const int x, const int y, const eRgbChannel rgbChannel)
	{
		assert(mImageType == eImageType::UInt8RGB && "image type must be UInt8RGB.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t channelIndex = static_cast<std::size_t>(rgbChannel);
		assert(channelIndex < static_cast<std::size_t>(mChannelCount) && "rgbChannel is out of range.");

		const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow) + static_cast<std::size_t>(x) * static_cast<std::size_t>(mChannelCount) + channelIndex;

		return mPixels[index];
	}

	const std::uint8_t& Image::GetRgbPixel(const int x, const int y, const eRgbChannel rgbChannel) const
	{
		assert(mImageType == eImageType::UInt8RGB && "image type must be UInt8RGB.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t channelIndex = static_cast<std::size_t>(rgbChannel);
		assert(channelIndex < static_cast<std::size_t>(mChannelCount) && "rgbChannel is out of range.");

		const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow) + static_cast<std::size_t>(x) * static_cast<std::size_t>(mChannelCount) + channelIndex;

		return mPixels[index];
	}
}
