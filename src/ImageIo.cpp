#include <cctype>
#include <filesystem>
#include <optional>
#include <string>

#include "PgmImageIo.h"
#include "PpmImageIo.h"
#include "minicv/ImageIo.h"

namespace minicv
{
	namespace
	{
		std::string GetLowercaseExtension(const std::filesystem::path& filePath)
		{
			std::string extension = filePath.extension().string();
			for (char& character : extension)
			{
				character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
			}

			return extension;
		}
	}

	std::optional<Image> TryLoadImage(const std::filesystem::path& filePath)
	{
		const std::string extension = GetLowercaseExtension(filePath);

		if (extension == ".pgm")
		{
			return netpbm::TryLoadPgmImage(filePath);
		}

		if (extension == ".ppm")
		{
			return netpbm::TryLoadPpmImage(filePath);
		}

		return std::nullopt;
	}

	bool TrySaveImage(const Image& image, const std::filesystem::path& filePath)
	{
		const std::string extension = GetLowercaseExtension(filePath);

		if (extension == ".pgm")
		{
			return netpbm::TrySavePgmImage(image, filePath);
		}

		if (extension == ".ppm")
		{
			return netpbm::TrySavePpmImage(image, filePath);
		}

		return false;
	}
}
