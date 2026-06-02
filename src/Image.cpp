#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
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
			case EImageType::UINT8_GRAYSCALE:
				return 1;
			case EImageType::UINT8_RGB:
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

			assert(widthSize <= static_cast<std::size_t>(std::numeric_limits<int>::max()) / channelCountSize && "bytes per row exceeds int range.");

			return static_cast<int>(widthSize * channelCountSize);
		}

		std::size_t CalculateByteCount(const int height, const int bytesPerRow)
		{
			assert(height >= 0 && "height must not be negative.");
			assert(bytesPerRow >= 0 && "bytes per row must not be negative.");

			const std::size_t heightSize = static_cast<std::size_t>(height);
			const std::size_t bytesPerRowSize = static_cast<std::size_t>(bytesPerRow);

			assert((bytesPerRowSize == 0 || heightSize <= std::numeric_limits<std::size_t>::max() / bytesPerRowSize) && "image byte count overflow.");

			return heightSize * bytesPerRowSize;
		}
	}

	Image::Image()
		: Image(0, 0, EImageType::UINT8_GRAYSCALE)
	{
	}

	Image::Image(const int width, const int height)
		: Image(width, height, EImageType::UINT8_GRAYSCALE)
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

	Image::Image(const Size size)
		: Image(size.Width, size.Height, EImageType::UINT8_GRAYSCALE)
	{
	}

	Image::Image(const Size size, const EImageType imageType)
		: Image(size.Width, size.Height, imageType)
	{
	}

	void Image::Create(const int width, const int height)
	{
		Create(width, height, EImageType::UINT8_GRAYSCALE);
	}

	void Image::Create(const int width, const int height, const EImageType imageType)
	{
		*this = Image(width, height, imageType);
	}

	void Image::Create(const Size size)
	{
		Create(size.Width, size.Height, EImageType::UINT8_GRAYSCALE);
	}

	void Image::Create(const Size size, const EImageType imageType)
	{
		Create(size.Width, size.Height, imageType);
	}

	void Image::Fill(const std::uint8_t value)
	{
		std::fill(mPixels.begin(), mPixels.end(), value);
	}

	void Image::FillRgb(const std::uint8_t red, const std::uint8_t green, const std::uint8_t blue)
	{
		assert(mImageType == EImageType::UINT8_RGB && "image type must be UINT8_RGB.");

		for (std::size_t index = 0; index < mPixels.size(); index += static_cast<std::size_t>(mChannelCount))
		{
			mPixels[index + static_cast<std::size_t>(ERgbChannel::RED)] = red;
			mPixels[index + static_cast<std::size_t>(ERgbChannel::GREEN)] = green;
			mPixels[index + static_cast<std::size_t>(ERgbChannel::BLUE)] = blue;
		}
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

	Size Image::GetSize() const
	{
		return Size{ mWidth, mHeight };
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

	bool Image::HasSameSize(const Image& other) const
	{
		return mWidth == other.mWidth && mHeight == other.mHeight;
	}

	bool Image::HasSameShape(const Image& other) const
	{
		return HasSameSize(other) && mImageType == other.mImageType;
	}

	bool Image::Contains(const Point point) const
	{
		return point.X >= 0 && point.X < mWidth && point.Y >= 0 && point.Y < mHeight;
	}

	bool Image::Contains(const Rect region) const
	{
		if (region.X < 0 || region.Y < 0 || region.Width < 0 || region.Height < 0)
		{
			return false;
		}

		const std::size_t imageWidth = static_cast<std::size_t>(mWidth);
		const std::size_t imageHeight = static_cast<std::size_t>(mHeight);
		const std::size_t regionX = static_cast<std::size_t>(region.X);
		const std::size_t regionY = static_cast<std::size_t>(region.Y);
		const std::size_t regionWidth = static_cast<std::size_t>(region.Width);
		const std::size_t regionHeight = static_cast<std::size_t>(region.Height);

		return regionX <= imageWidth && regionWidth <= imageWidth - regionX && regionY <= imageHeight && regionHeight <= imageHeight - regionY;
	}

	Image Image::Clone() const
	{
		return *this;
	}

	Image Image::ExtractRegion(const Rect region) const
	{
		assert(Contains(region) && "region must be inside the image.");

		Image extractedImage(region.Width, region.Height, mImageType);
		const std::size_t sourceColumnOffset = static_cast<std::size_t>(region.X) * static_cast<std::size_t>(mChannelCount);
		const std::size_t bytesPerRegionRow = static_cast<std::size_t>(region.Width) * static_cast<std::size_t>(mChannelCount);

		if (bytesPerRegionRow == 0)
		{
			return extractedImage;
		}

		for (int y = 0; y < region.Height; ++y)
		{
			const std::size_t sourceRowOffset = static_cast<std::size_t>(region.Y + y) * static_cast<std::size_t>(mBytesPerRow);
			const std::size_t destinationRowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(extractedImage.mBytesPerRow);
			const std::size_t sourceIndex = sourceRowOffset + sourceColumnOffset;

			std::memcpy(extractedImage.mPixels.data() + destinationRowOffset, mPixels.data() + sourceIndex, bytesPerRegionRow);
		}

		return extractedImage;
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
		assert(mImageType == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow);
		const std::size_t index = rowOffset + static_cast<std::size_t>(x);

		return mPixels[index];
	}

	const std::uint8_t& Image::GetGrayscalePixel(const int x, const int y) const
	{
		assert(mImageType == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow);
		const std::size_t index = rowOffset + static_cast<std::size_t>(x);

		return mPixels[index];
	}

	std::uint8_t& Image::GetRgbPixel(const int x, const int y, const ERgbChannel rgbChannel)
	{
		assert(mImageType == EImageType::UINT8_RGB && "image type must be UINT8_RGB.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t channelIndex = static_cast<std::size_t>(rgbChannel);
		assert(channelIndex < static_cast<std::size_t>(mChannelCount) && "rgbChannel is out of range.");

		const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow);
		const std::size_t columnOffset = static_cast<std::size_t>(x) * static_cast<std::size_t>(mChannelCount);
		const std::size_t index = rowOffset + columnOffset + channelIndex;

		return mPixels[index];
	}

	const std::uint8_t& Image::GetRgbPixel(const int x, const int y, const ERgbChannel rgbChannel) const
	{
		assert(mImageType == EImageType::UINT8_RGB && "image type must be UINT8_RGB.");
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");

		const std::size_t channelIndex = static_cast<std::size_t>(rgbChannel);
		assert(channelIndex < static_cast<std::size_t>(mChannelCount) && "rgbChannel is out of range.");

		const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(mBytesPerRow);
		const std::size_t columnOffset = static_cast<std::size_t>(x) * static_cast<std::size_t>(mChannelCount);
		const std::size_t index = rowOffset + columnOffset + channelIndex;

		return mPixels[index];
	}
}
