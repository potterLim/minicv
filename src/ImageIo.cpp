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
		std::filesystem::path GetLowercaseExtension(const std::filesystem::path& filePath)
		{
			std::filesystem::path::string_type extension = filePath.extension().native();
			for (std::filesystem::path::value_type& character : extension)
			{
				// Supported extensions are ASCII; leave all other native characters unchanged.
				if (character >= 'A' && character <= 'Z')
				{
					character = static_cast<std::filesystem::path::value_type>(character + ('a' - 'A'));
				}
			}

			return extension;
		}
	}

	std::optional<Image> TryLoadImage(const std::filesystem::path& filePath)
	{
		const std::filesystem::path extension = GetLowercaseExtension(filePath);

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
		const std::filesystem::path extension = GetLowercaseExtension(filePath);

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
