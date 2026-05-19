#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "minicv/EImageType.h"
#include "minicv/ERgbChannel.h"

namespace minicv
{
	class Image
	{
	public:
		Image();
		Image(const int width, const int height);
		Image(const int width, const int height, const EImageType imageType);

		[[nodiscard]] bool IsEmpty() const;

		[[nodiscard]] int GetWidth() const;
		[[nodiscard]] int GetHeight() const;
		[[nodiscard]] int GetChannelCount() const;
		[[nodiscard]] int GetBytesPerRow() const;
		[[nodiscard]] EImageType GetImageType() const;

		[[nodiscard]] std::size_t GetPixelCount() const;
		[[nodiscard]] std::size_t GetByteCount() const;

		[[nodiscard]] std::uint8_t* GetPixelData();
		[[nodiscard]] const std::uint8_t* GetPixelData() const;

		[[nodiscard]] std::uint8_t& GetGrayscalePixel(const int x, const int y);
		[[nodiscard]] const std::uint8_t& GetGrayscalePixel(const int x, const int y) const;

		[[nodiscard]] std::uint8_t& GetRgbPixel(const int x, const int y, const ERgbChannel rgbChannel);
		[[nodiscard]] const std::uint8_t& GetRgbPixel(const int x, const int y, const ERgbChannel rgbChannel) const;

	private:
		int mWidth;
		int mHeight;
		int mChannelCount;
		int mBytesPerRow;
		EImageType mImageType;
		std::vector<std::uint8_t> mPixels;
	};
}
