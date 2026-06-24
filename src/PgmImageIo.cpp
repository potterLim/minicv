#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

#include "NetpbmImageIo.h"
#include "PgmImageIo.h"
#include "minicv/EImageType.h"

namespace minicv::netpbm
{
	namespace
	{
		bool TryReadAsciiPixels(std::istream& inputStream, Image* outImage)
		{
			assert(outImage != nullptr && "outImage must not be null.");

			for (int y = 0; y < outImage->GetHeight(); ++y)
			{
				for (int x = 0; x < outImage->GetWidth(); ++x)
				{
					const std::optional<std::uint8_t> pixelValue = TryReadPixelValue(inputStream);
					if (!pixelValue.has_value())
					{
						return false;
					}

					outImage->GetGrayscalePixel(x, y) = *pixelValue;
				}
			}

			return true;
		}
	}

	std::optional<Image> TryLoadPgmImage(const std::filesystem::path& filePath)
	{
		std::ifstream inputStream(filePath, std::ios::binary);
		if (!inputStream.is_open())
		{
			return std::nullopt;
		}

		const std::optional<NetpbmHeader> header = TryReadNetpbmHeader(inputStream);
		if (!header.has_value() || !IsPgmFormat(header->Format))
		{
			return std::nullopt;
		}

		std::optional<Image> image = TryCreateImage(*header);
		if (!image.has_value())
		{
			return std::nullopt;
		}

		Image& loadedImage = *image;
		if (IsAsciiFormat(header->Format))
		{
			if (!TryReadAsciiPixels(inputStream, &loadedImage))
			{
				return std::nullopt;
			}
		}
		else if (!TryReadBinaryPixels(inputStream, &loadedImage))
		{
			return std::nullopt;
		}

		return image;
	}

	bool TrySavePgmImage(const Image& image, const std::filesystem::path& filePath)
	{
		if (image.GetImageType() != EImageType::UINT8_GRAYSCALE || image.IsEmpty())
		{
			return false;
		}

		std::ofstream outputStream(filePath, std::ios::binary);
		if (!outputStream.is_open())
		{
			return false;
		}

		if (!TryWriteNetpbmHeader(outputStream, ENetpbmFormat::PGM_BINARY, image))
		{
			return false;
		}

		return TryWriteBinaryPixels(outputStream, image);
	}
}
