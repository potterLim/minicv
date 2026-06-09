#pragma once

#include <filesystem>
#include <optional>

#include "minicv/EPgmFormat.h"
#include "minicv/Image.h"

namespace minicv
{
	[[nodiscard]] std::optional<Image> TryLoadPgmImage(const std::filesystem::path& filePath);
	[[nodiscard]] bool TrySavePgmImage(const Image& image, const std::filesystem::path& filePath, const EPgmFormat pgmFormat);
}
