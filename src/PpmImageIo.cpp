#include <cassert>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>

#include "minicv/PpmImageIo.h"

namespace minicv
{
	namespace
	{
		constexpr int PPM_CHANNEL_COUNT = 3;
		constexpr int PPM_MAX_PIXEL_VALUE = 255;
		constexpr std::size_t PPM_MAX_IMAGE_BYTE_COUNT = static_cast<std::size_t>(std::numeric_limits<int>::max());

		bool CanCreatePpmImage(const int width, const int height)
		{
			if (width <= 0 || height <= 0)
			{
				return false;
			}

			const std::size_t widthSize = static_cast<std::size_t>(width);
			const std::size_t heightSize = static_cast<std::size_t>(height);
			const std::size_t channelCount = static_cast<std::size_t>(PPM_CHANNEL_COUNT);

			const bool canCalculateBytesPerRow = widthSize <= static_cast<std::size_t>(std::numeric_limits<int>::max()) / channelCount;
			if (!canCalculateBytesPerRow)
			{
				return false;
			}

			const std::size_t bytesPerRow = widthSize * channelCount;
			const bool canCalculateByteCount = heightSize <= std::numeric_limits<std::size_t>::max() / bytesPerRow;
			if (!canCalculateByteCount)
			{
				return false;
			}

			const std::size_t byteCount = heightSize * bytesPerRow;
			return byteCount <= PPM_MAX_IMAGE_BYTE_COUNT;
		}

		std::optional<Image> TryCreatePpmImage(const int width, const int height)
		{
			if (!CanCreatePpmImage(width, height))
			{
				return std::nullopt;
			}

			try
			{
				return Image(width, height, EImageType::UINT8_RGB);
			}
			catch (const std::bad_alloc&)
			{
				return std::nullopt;
			}
			catch (const std::length_error&)
			{
				return std::nullopt;
			}
		}

		bool ReadToken(std::istream& inputStream, std::string* outToken)
		{
			assert(outToken != nullptr && "outToken must not be null.");

			outToken->clear();

			char character = '\0';
			while (inputStream.get(character))
			{
				if (std::isspace(static_cast<unsigned char>(character)) != 0)
				{
					continue;
				}

				if (character == '#')
				{
					inputStream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					continue;
				}

				outToken->push_back(character);
				break;
			}

			if (outToken->empty())
			{
				return false;
			}

			while (inputStream.get(character))
			{
				if (std::isspace(static_cast<unsigned char>(character)) != 0)
				{
					if (character == '\r' && inputStream.peek() == '\n')
					{
						inputStream.get(character);
					}

					return true;
				}

				if (character == '#')
				{
					inputStream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					return true;
				}

				outToken->push_back(character);
			}

			return true;
		}

		bool TryParseInt(const std::string& text, int* outValue)
		{
			assert(outValue != nullptr && "outValue must not be null.");

			int value = 0;
			const char* const begin = text.data();
			const char* const end = begin + text.size();
			const std::from_chars_result result = std::from_chars(begin, end, value);

			if (result.ec != std::errc{} || result.ptr != end)
			{
				return false;
			}

			*outValue = value;
			return true;
		}

		bool TryReadIntToken(std::istream& inputStream, int* outValue)
		{
			assert(outValue != nullptr && "outValue must not be null.");

			std::string token;
			if (!ReadToken(inputStream, &token))
			{
				return false;
			}

			return TryParseInt(token, outValue);
		}

		bool TryReadPpmFormat(std::istream& inputStream, EPpmFormat* outPpmFormat)
		{
			assert(outPpmFormat != nullptr && "outPpmFormat must not be null.");

			std::string magicNumber;
			if (!ReadToken(inputStream, &magicNumber))
			{
				return false;
			}

			if (magicNumber == "P3")
			{
				*outPpmFormat = EPpmFormat::ASCII;
				return true;
			}

			if (magicNumber == "P6")
			{
				*outPpmFormat = EPpmFormat::BINARY;
				return true;
			}

			return false;
		}

		bool TryReadPpmHeader(std::istream& inputStream, EPpmFormat* outPpmFormat, int* outWidth, int* outHeight)
		{
			assert(outPpmFormat != nullptr && "outPpmFormat must not be null.");
			assert(outWidth != nullptr && "outWidth must not be null.");
			assert(outHeight != nullptr && "outHeight must not be null.");

			if (!TryReadPpmFormat(inputStream, outPpmFormat))
			{
				return false;
			}

			if (!TryReadIntToken(inputStream, outWidth))
			{
				return false;
			}

			if (!TryReadIntToken(inputStream, outHeight))
			{
				return false;
			}

			int maxPixelValue = 0;
			if (!TryReadIntToken(inputStream, &maxPixelValue))
			{
				return false;
			}

			return maxPixelValue == PPM_MAX_PIXEL_VALUE && CanCreatePpmImage(*outWidth, *outHeight);
		}

		bool TryReadAsciiChannel(std::istream& inputStream, std::uint8_t* outChannelValue)
		{
			assert(outChannelValue != nullptr && "outChannelValue must not be null.");

			int channelValue = 0;
			if (!TryReadIntToken(inputStream, &channelValue) || channelValue < 0 || channelValue > PPM_MAX_PIXEL_VALUE)
			{
				return false;
			}

			*outChannelValue = static_cast<std::uint8_t>(channelValue);
			return true;
		}

		bool TryReadAsciiPixels(std::istream& inputStream, Image* outImage)
		{
			assert(outImage != nullptr && "outImage must not be null.");

			for (int y = 0; y < outImage->GetHeight(); ++y)
			{
				for (int x = 0; x < outImage->GetWidth(); ++x)
				{
					std::uint8_t red = 0;
					std::uint8_t green = 0;
					std::uint8_t blue = 0;

					if (!TryReadAsciiChannel(inputStream, &red))
					{
						return false;
					}

					if (!TryReadAsciiChannel(inputStream, &green))
					{
						return false;
					}

					if (!TryReadAsciiChannel(inputStream, &blue))
					{
						return false;
					}

					outImage->GetRgbPixel(x, y, ERgbChannel::RED) = red;
					outImage->GetRgbPixel(x, y, ERgbChannel::GREEN) = green;
					outImage->GetRgbPixel(x, y, ERgbChannel::BLUE) = blue;
				}
			}

			return true;
		}

		bool TryReadBinaryPixels(std::istream& inputStream, Image* outImage)
		{
			assert(outImage != nullptr && "outImage must not be null.");

			const std::size_t byteCount = outImage->GetByteCount();
			if (byteCount > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()))
			{
				return false;
			}

			inputStream.read(reinterpret_cast<char*>(outImage->GetPixelData()), static_cast<std::streamsize>(byteCount));
			return static_cast<std::size_t>(inputStream.gcount()) == byteCount;
		}

		bool TryWriteAsciiPixels(std::ostream& outputStream, const Image& image)
		{
			for (int y = 0; y < image.GetHeight(); ++y)
			{
				for (int x = 0; x < image.GetWidth(); ++x)
				{
					if (x > 0)
					{
						outputStream << ' ';
					}

					outputStream << static_cast<int>(image.GetRgbPixel(x, y, ERgbChannel::RED)) << ' ';
					outputStream << static_cast<int>(image.GetRgbPixel(x, y, ERgbChannel::GREEN)) << ' ';
					outputStream << static_cast<int>(image.GetRgbPixel(x, y, ERgbChannel::BLUE));
				}

				outputStream << '\n';
			}

			return outputStream.good();
		}

		bool TryWriteBinaryPixels(std::ostream& outputStream, const Image& image)
		{
			const std::size_t byteCount = image.GetByteCount();
			if (byteCount > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()))
			{
				return false;
			}

			outputStream.write(reinterpret_cast<const char*>(image.GetPixelData()), static_cast<std::streamsize>(byteCount));
			return outputStream.good();
		}
	}

	std::optional<Image> TryLoadPpmImage(const std::filesystem::path& filePath)
	{
		std::ifstream inputStream(filePath, std::ios::binary);
		if (!inputStream.is_open())
		{
			return std::nullopt;
		}

		EPpmFormat ppmFormat = EPpmFormat::ASCII;
		int width = 0;
		int height = 0;
		if (!TryReadPpmHeader(inputStream, &ppmFormat, &width, &height))
		{
			return std::nullopt;
		}

		std::optional<Image> image = TryCreatePpmImage(width, height);
		if (!image.has_value())
		{
			return std::nullopt;
		}

		Image& loadedImage = *image;
		switch (ppmFormat)
		{
		case EPpmFormat::ASCII:
			if (!TryReadAsciiPixels(inputStream, &loadedImage))
			{
				return std::nullopt;
			}
			break;
		case EPpmFormat::BINARY:
			if (!TryReadBinaryPixels(inputStream, &loadedImage))
			{
				return std::nullopt;
			}
			break;
		default:
			assert(false && "Unsupported PPM format.");
			return std::nullopt;
		}

		return image;
	}

	bool TrySavePpmImage(const Image& image, const std::filesystem::path& filePath, const EPpmFormat ppmFormat)
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

		switch (ppmFormat)
		{
		case EPpmFormat::ASCII:
			outputStream << "P3\n";
			break;
		case EPpmFormat::BINARY:
			outputStream << "P6\n";
			break;
		default:
			assert(false && "Unsupported PPM format.");
			return false;
		}

		outputStream << image.GetWidth() << ' ' << image.GetHeight() << '\n';
		outputStream << PPM_MAX_PIXEL_VALUE << '\n';

		switch (ppmFormat)
		{
		case EPpmFormat::ASCII:
			return TryWriteAsciiPixels(outputStream, image);
		case EPpmFormat::BINARY:
			return TryWriteBinaryPixels(outputStream, image);
		default:
			assert(false && "Unsupported PPM format.");
			return false;
		}
	}
}
