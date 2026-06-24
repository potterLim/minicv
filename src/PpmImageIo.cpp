#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

#include "NetpbmImageIo.h"
#include "PpmImageIo.h"
#include "minicv/EImageType.h"
#include "minicv/ERgbChannel.h"

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
					const std::optional<std::uint8_t> red = TryReadPixelValue(inputStream);
					const std::optional<std::uint8_t> green = TryReadPixelValue(inputStream);
					const std::optional<std::uint8_t> blue = TryReadPixelValue(inputStream);

					if (!red.has_value() || !green.has_value() || !blue.has_value())
					{
						return false;
					}

					outImage->GetRgbPixel(x, y, ERgbChannel::RED) = *red;
					outImage->GetRgbPixel(x, y, ERgbChannel::GREEN) = *green;
					outImage->GetRgbPixel(x, y, ERgbChannel::BLUE) = *blue;
				}
			}

			return true;
		}
	}

	std::optional<Image> TryLoadPpmImage(const std::filesystem::path& filePath)
	{
		std::ifstream inputStream(filePath, std::ios::binary);
		if (!inputStream.is_open())
		{
			return std::nullopt;
		}

		const std::optional<NetpbmHeader> header = TryReadNetpbmHeader(inputStream);
		if (!header.has_value() || !IsPpmFormat(header->Format))
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

	bool TrySavePpmImage(const Image& image, const std::filesystem::path& filePath)
	{
		if (image.GetImageType() != EImageType::UINT8_RGB || image.IsEmpty())
		{
			return false;
		}

		std::ofstream outputStream(filePath, std::ios::binary);
		if (!outputStream.is_open())
		{
			return false;
		}

		if (!TryWriteNetpbmHeader(outputStream, ENetpbmFormat::PPM_BINARY, image))
		{
			return false;
		}

		return TryWriteBinaryPixels(outputStream, image);
	}
}
