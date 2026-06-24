#pragma once

#include <cstdint>
#include <iosfwd>
#include <optional>

#include "minicv/Image.h"

namespace minicv::netpbm
{
	enum class ENetpbmFormat
	{
		PGM_ASCII,
		PGM_BINARY,
		PPM_ASCII,
		PPM_BINARY
	};

	struct NetpbmHeader
	{
		ENetpbmFormat Format;
		int Width;
		int Height;
	};

	[[nodiscard]] bool IsPgmFormat(const ENetpbmFormat netpbmFormat);
	[[nodiscard]] bool IsPpmFormat(const ENetpbmFormat netpbmFormat);
	[[nodiscard]] bool IsAsciiFormat(const ENetpbmFormat netpbmFormat);

	[[nodiscard]] std::optional<NetpbmHeader> TryReadNetpbmHeader(std::istream& inputStream);
	[[nodiscard]] std::optional<int> TryReadIntToken(std::istream& inputStream);
	[[nodiscard]] std::optional<std::uint8_t> TryReadPixelValue(std::istream& inputStream);
	[[nodiscard]] std::optional<Image> TryCreateImage(const NetpbmHeader& header);

	[[nodiscard]] bool TryReadBinaryPixels(std::istream& inputStream, Image* outImage);
	[[nodiscard]] bool TryWriteNetpbmHeader(std::ostream& outputStream, const ENetpbmFormat netpbmFormat, const Image& image);
	[[nodiscard]] bool TryWriteBinaryPixels(std::ostream& outputStream, const Image& image);
}
