#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "minicv/EImageType.h"
#include "minicv/ERgbChannel.h"
#include "minicv/Point.h"
#include "minicv/Rect.h"
#include "minicv/Size.h"

namespace minicv
{
	class Image
	{
	public:
		Image();
		Image(const int width, const int height);
		Image(const int width, const int height, const EImageType imageType);
		Image(const Size size);
		Image(const Size size, const EImageType imageType);

		void Create(const int width, const int height);
		void Create(const int width, const int height, const EImageType imageType);
		void Create(const Size size);
		void Create(const Size size, const EImageType imageType);

		void Fill(const std::uint8_t value);
		void FillRgb(const std::uint8_t red, const std::uint8_t green, const std::uint8_t blue);

		[[nodiscard]] bool IsEmpty() const;

		[[nodiscard]] int GetWidth() const;
		[[nodiscard]] int GetHeight() const;
		[[nodiscard]] Size GetSize() const;
		[[nodiscard]] int GetChannelCount() const;
		[[nodiscard]] int GetBytesPerRow() const;
		[[nodiscard]] EImageType GetImageType() const;

		[[nodiscard]] std::size_t GetPixelCount() const;
		[[nodiscard]] std::size_t GetByteCount() const;

		[[nodiscard]] bool HasSameSize(const Image& other) const;
		[[nodiscard]] bool HasSameShape(const Image& other) const;
		[[nodiscard]] bool Contains(const Point point) const;
		[[nodiscard]] bool Contains(const Rect region) const;

		[[nodiscard]] Image Clone() const;
		[[nodiscard]] Image ExtractRegion(const Rect region) const;

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
