#pragma once

#include <filesystem>
#include <optional>

#include "minicv/EPpmFormat.h"
#include "minicv/Image.h"

namespace minicv
{
	[[nodiscard]] std::optional<Image> TryLoadPpmImage(const std::filesystem::path& filePath);
	[[nodiscard]] bool TrySavePpmImage(const Image& image, const std::filesystem::path& filePath, const EPpmFormat ppmFormat);
}
