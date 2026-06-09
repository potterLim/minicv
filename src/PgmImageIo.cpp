#include <cassert>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <optional>
#include <string>
#include <system_error>

#include "minicv/PgmImageIo.h"

namespace minicv
{
	namespace
	{
		constexpr int PGM_MAX_PIXEL_VALUE = 255;

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

		bool TryReadPgmFormat(std::istream& inputStream, EPgmFormat* outPgmFormat)
		{
			assert(outPgmFormat != nullptr && "outPgmFormat must not be null.");

			std::string magicNumber;
			if (!ReadToken(inputStream, &magicNumber))
			{
				return false;
			}

			if (magicNumber == "P2")
			{
				*outPgmFormat = EPgmFormat::ASCII;
				return true;
			}

			if (magicNumber == "P5")
			{
				*outPgmFormat = EPgmFormat::BINARY;
				return true;
			}

			return false;
		}

		bool TryReadPgmHeader(std::istream& inputStream, EPgmFormat* outPgmFormat, int* outWidth, int* outHeight)
		{
			assert(outPgmFormat != nullptr && "outPgmFormat must not be null.");
			assert(outWidth != nullptr && "outWidth must not be null.");
			assert(outHeight != nullptr && "outHeight must not be null.");

			if (!TryReadPgmFormat(inputStream, outPgmFormat))
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

			return *outWidth > 0 && *outHeight > 0 && maxPixelValue == PGM_MAX_PIXEL_VALUE;
		}

		bool TryReadAsciiPixels(std::istream& inputStream, Image* outImage)
		{
			assert(outImage != nullptr && "outImage must not be null.");

			for (int y = 0; y < outImage->GetHeight(); ++y)
			{
				for (int x = 0; x < outImage->GetWidth(); ++x)
				{
					int pixelValue = 0;
					if (!TryReadIntToken(inputStream, &pixelValue) || pixelValue < 0 || pixelValue > PGM_MAX_PIXEL_VALUE)
					{
						return false;
					}

					outImage->GetGrayscalePixel(x, y) = static_cast<std::uint8_t>(pixelValue);
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

					outputStream << static_cast<int>(image.GetGrayscalePixel(x, y));
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

	std::optional<Image> TryLoadPgmImage(const std::filesystem::path& filePath)
	{
		std::ifstream inputStream(filePath, std::ios::binary);
		if (!inputStream.is_open())
		{
			return std::nullopt;
		}

		EPgmFormat pgmFormat = EPgmFormat::ASCII;
		int width = 0;
		int height = 0;
		if (!TryReadPgmHeader(inputStream, &pgmFormat, &width, &height))
		{
			return std::nullopt;
		}

		Image image(width, height, EImageType::UINT8_GRAYSCALE);
		switch (pgmFormat)
		{
		case EPgmFormat::ASCII:
			if (!TryReadAsciiPixels(inputStream, &image))
			{
				return std::nullopt;
			}
			break;
		case EPgmFormat::BINARY:
			if (!TryReadBinaryPixels(inputStream, &image))
			{
				return std::nullopt;
			}
			break;
		default:
			assert(false && "Unsupported PGM format.");
			return std::nullopt;
		}

		return image;
	}

	bool TrySavePgmImage(const Image& image, const std::filesystem::path& filePath, const EPgmFormat pgmFormat)
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

		switch (pgmFormat)
		{
		case EPgmFormat::ASCII:
			outputStream << "P2\n";
			break;
		case EPgmFormat::BINARY:
			outputStream << "P5\n";
			break;
		default:
			assert(false && "Unsupported PGM format.");
			return false;
		}

		outputStream << image.GetWidth() << ' ' << image.GetHeight() << '\n';
		outputStream << PGM_MAX_PIXEL_VALUE << '\n';

		switch (pgmFormat)
		{
		case EPgmFormat::ASCII:
			return TryWriteAsciiPixels(outputStream, image);
		case EPgmFormat::BINARY:
			return TryWriteBinaryPixels(outputStream, image);
		default:
			assert(false && "Unsupported PGM format.");
			return false;
		}
	}
}
