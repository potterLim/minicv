#pragma once

#include <filesystem>
#include <optional>

#include "minicv/Image.h"

namespace minicv
{
	[[nodiscard]] std::optional<Image> TryLoadImage(const std::filesystem::path& filePath);
	[[nodiscard]] bool TrySaveImage(const Image& image, const std::filesystem::path& filePath);
}
