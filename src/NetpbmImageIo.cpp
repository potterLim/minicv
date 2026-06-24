#include <cassert>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <istream>
#include <limits>
#include <new>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <system_error>

#include "NetpbmImageIo.h"
#include "minicv/EImageType.h"

namespace minicv::netpbm
{
	namespace
	{
		constexpr int NETPBM_MAX_PIXEL_VALUE = 255;
		constexpr std::size_t NETPBM_MAX_IMAGE_BYTE_COUNT = static_cast<std::size_t>(std::numeric_limits<int>::max());

		int GetChannelCount(const ENetpbmFormat netpbmFormat)
		{
			switch (netpbmFormat)
			{
			case ENetpbmFormat::PGM_ASCII:
			case ENetpbmFormat::PGM_BINARY:
				return 1;
			case ENetpbmFormat::PPM_ASCII:
			case ENetpbmFormat::PPM_BINARY:
				return 3;
			default:
				assert(false && "Unsupported Netpbm format.");
				return 0;
			}
		}

		EImageType GetImageType(const ENetpbmFormat netpbmFormat)
		{
			switch (netpbmFormat)
			{
			case ENetpbmFormat::PGM_ASCII:
			case ENetpbmFormat::PGM_BINARY:
				return EImageType::UINT8_GRAYSCALE;
			case ENetpbmFormat::PPM_ASCII:
			case ENetpbmFormat::PPM_BINARY:
				return EImageType::UINT8_RGB;
			default:
				assert(false && "Unsupported Netpbm format.");
				return EImageType::UINT8_GRAYSCALE;
			}
		}

		const char* GetMagicNumber(const ENetpbmFormat netpbmFormat)
		{
			switch (netpbmFormat)
			{
			case ENetpbmFormat::PGM_ASCII:
				return "P2";
			case ENetpbmFormat::PGM_BINARY:
				return "P5";
			case ENetpbmFormat::PPM_ASCII:
				return "P3";
			case ENetpbmFormat::PPM_BINARY:
				return "P6";
			default:
				assert(false && "Unsupported Netpbm format.");
				return "";
			}
		}

		std::optional<ENetpbmFormat> TryParseFormat(const std::string& magicNumber)
		{
			if (magicNumber == "P2")
			{
				return ENetpbmFormat::PGM_ASCII;
			}

			if (magicNumber == "P5")
			{
				return ENetpbmFormat::PGM_BINARY;
			}

			if (magicNumber == "P3")
			{
				return ENetpbmFormat::PPM_ASCII;
			}

			if (magicNumber == "P6")
			{
				return ENetpbmFormat::PPM_BINARY;
			}

			return std::nullopt;
		}

		bool CanCreateImage(const NetpbmHeader& header)
		{
			if (header.Width <= 0 || header.Height <= 0)
			{
				return false;
			}

			const std::size_t width = static_cast<std::size_t>(header.Width);
			const std::size_t height = static_cast<std::size_t>(header.Height);
			const std::size_t channelCount = static_cast<std::size_t>(GetChannelCount(header.Format));

			const bool canCalculateBytesPerRow = width <= static_cast<std::size_t>(std::numeric_limits<int>::max()) / channelCount;
			if (!canCalculateBytesPerRow)
			{
				return false;
			}

			const std::size_t bytesPerRow = width * channelCount;
			const bool canCalculateByteCount = height <= std::numeric_limits<std::size_t>::max() / bytesPerRow;
			if (!canCalculateByteCount)
			{
				return false;
			}

			const std::size_t byteCount = height * bytesPerRow;
			return byteCount <= NETPBM_MAX_IMAGE_BYTE_COUNT;
		}

		bool TryReadToken(std::istream& inputStream, std::string* outToken)
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

		std::optional<int> TryParseInt(const std::string& text)
		{
			int value = 0;
			const char* const begin = text.data();
			const char* const end = begin + text.size();
			const std::from_chars_result parseResult = std::from_chars(begin, end, value);

			if (parseResult.ec != std::errc{} || parseResult.ptr != end)
			{
				return std::nullopt;
			}

			return value;
		}
	}

	bool IsPgmFormat(const ENetpbmFormat netpbmFormat)
	{
		return netpbmFormat == ENetpbmFormat::PGM_ASCII || netpbmFormat == ENetpbmFormat::PGM_BINARY;
	}

	bool IsPpmFormat(const ENetpbmFormat netpbmFormat)
	{
		return netpbmFormat == ENetpbmFormat::PPM_ASCII || netpbmFormat == ENetpbmFormat::PPM_BINARY;
	}

	bool IsAsciiFormat(const ENetpbmFormat netpbmFormat)
	{
		return netpbmFormat == ENetpbmFormat::PGM_ASCII || netpbmFormat == ENetpbmFormat::PPM_ASCII;
	}

	std::optional<NetpbmHeader> TryReadNetpbmHeader(std::istream& inputStream)
	{
		std::string magicNumber;
		if (!TryReadToken(inputStream, &magicNumber))
		{
			return std::nullopt;
		}

		const std::optional<ENetpbmFormat> netpbmFormat = TryParseFormat(magicNumber);
		if (!netpbmFormat.has_value())
		{
			return std::nullopt;
		}

		const std::optional<int> width = TryReadIntToken(inputStream);
		if (!width.has_value())
		{
			return std::nullopt;
		}

		const std::optional<int> height = TryReadIntToken(inputStream);
		if (!height.has_value())
		{
			return std::nullopt;
		}

		const std::optional<int> maxPixelValue = TryReadIntToken(inputStream);
		if (!maxPixelValue.has_value() || *maxPixelValue != NETPBM_MAX_PIXEL_VALUE)
		{
			return std::nullopt;
		}

		const NetpbmHeader header{ *netpbmFormat, *width, *height };
		if (!CanCreateImage(header))
		{
			return std::nullopt;
		}

		return header;
	}

	std::optional<int> TryReadIntToken(std::istream& inputStream)
	{
		std::string token;
		if (!TryReadToken(inputStream, &token))
		{
			return std::nullopt;
		}

		return TryParseInt(token);
	}

	std::optional<std::uint8_t> TryReadPixelValue(std::istream& inputStream)
	{
		const std::optional<int> pixelValue = TryReadIntToken(inputStream);
		if (!pixelValue.has_value() || *pixelValue < 0 || *pixelValue > NETPBM_MAX_PIXEL_VALUE)
		{
			return std::nullopt;
		}

		return static_cast<std::uint8_t>(*pixelValue);
	}

	std::optional<Image> TryCreateImage(const NetpbmHeader& header)
	{
		if (!CanCreateImage(header))
		{
			return std::nullopt;
		}

		try
		{
			return Image(header.Width, header.Height, GetImageType(header.Format));
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

	bool TryWriteNetpbmHeader(std::ostream& outputStream, const ENetpbmFormat netpbmFormat, const Image& image)
	{
		outputStream << GetMagicNumber(netpbmFormat) << '\n';
		outputStream << image.GetWidth() << ' ' << image.GetHeight() << '\n';
		outputStream << NETPBM_MAX_PIXEL_VALUE << '\n';

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
