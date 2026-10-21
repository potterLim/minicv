#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <system_error>

#include "minicv/ConnectedComponents.h"
#include "minicv/ImageIo.h"
#include "minicv/ImageMorphology.h"
#include "minicv/ImageOperations.h"
#include "minicv/Point.h"
#include "minicv/Rect.h"

namespace
{
	constexpr int IMAGE_WIDTH = 40;
	constexpr int IMAGE_HEIGHT = 24;
	constexpr std::uint8_t BACKGROUND_VALUE = 30;
	constexpr std::uint8_t OBJECT_VALUE = 180;
	constexpr std::size_t MINIMUM_CANDIDATE_AREA = 20;
	constexpr minicv::Point BRIGHT_NOISE_POSITION{ 1, 20 };
	constexpr minicv::Point DARK_NOISE_POSITION{ 7, 7 };
	constexpr std::array OBJECT_REGIONS
	{
		minicv::Rect{ 3, 3, 9, 9 },
		minicv::Rect{ 19, 5, 7, 9 },
		minicv::Rect{ 32, 16, 3, 3 }
	};

	[[nodiscard]] minicv::Image CreateSourceImage()
	{
		minicv::Image image(IMAGE_WIDTH, IMAGE_HEIGHT);
		image.Fill(BACKGROUND_VALUE);
		for (const minicv::Rect region : OBJECT_REGIONS)
		{
			for (int y = region.Y; y < region.Y + region.Height; ++y)
			{
				for (int x = region.X; x < region.X + region.Width; ++x)
				{
					image.GetGrayscalePixel(x, y) = OBJECT_VALUE;
				}
			}
		}

		image.GetGrayscalePixel(BRIGHT_NOISE_POSITION.X, BRIGHT_NOISE_POSITION.Y) = OBJECT_VALUE;
		image.GetGrayscalePixel(DARK_NOISE_POSITION.X, DARK_NOISE_POSITION.Y) = BACKGROUND_VALUE;
		return image;
	}

	[[nodiscard]] minicv::Image CreateCandidateMask(const minicv::ConnectedComponents& components)
	{
		minicv::Image mask(components.GetSize());
		mask.Fill(0);
		for (int y = 0; y < mask.GetHeight(); ++y)
		{
			for (int x = 0; x < mask.GetWidth(); ++x)
			{
				const std::size_t label = components.GetLabel(x, y);
				if (label != 0 && components.GetComponents()[label - 1].Area >= MINIMUM_CANDIDATE_AREA)
				{
					mask.GetGrayscalePixel(x, y) = 255;
				}
			}
		}

		return mask;
	}

	[[nodiscard]] minicv::Image CreateExpectedCandidateMask()
	{
		minicv::Image mask(IMAGE_WIDTH, IMAGE_HEIGHT);
		mask.Fill(0);
		for (const minicv::Rect region : std::array{ OBJECT_REGIONS[0], OBJECT_REGIONS[1] })
		{
			for (int y = region.Y; y < region.Y + region.Height; ++y)
			{
				for (int x = region.X; x < region.X + region.Width; ++x)
				{
					mask.GetGrayscalePixel(x, y) = 255;
				}
			}
		}

		return mask;
	}

	[[nodiscard]] bool HasExpectedComponents(const minicv::ConnectedComponents& components)
	{
		if (components.GetComponentCount() != OBJECT_REGIONS.size())
		{
			return false;
		}

		for (std::size_t index = 0; index < OBJECT_REGIONS.size(); ++index)
		{
			const minicv::Rect region = OBJECT_REGIONS[index];
			const minicv::ConnectedComponent& component = components.GetComponents()[index];
			const std::size_t expectedArea = static_cast<std::size_t>(region.Width) * region.Height;
			if (component.Area != expectedArea ||
				component.BoundingBox.X != region.X || component.BoundingBox.Y != region.Y ||
				component.BoundingBox.Width != region.Width || component.BoundingBox.Height != region.Height ||
				component.CentroidX != region.X + (region.Width - 1) / 2.0 ||
				component.CentroidY != region.Y + (region.Height - 1) / 2.0)
			{
				return false;
			}
		}

		return true;
	}

	[[nodiscard]] bool TrySaveAndVerifyImage(const minicv::Image& image, const std::filesystem::path& filePath)
	{
		if (!minicv::TrySaveImage(image, filePath))
		{
			std::cerr << "Failed to save image: " << filePath << '\n';
			return false;
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);
		if (!loadedImage.has_value() || !loadedImage->HasSameContent(image))
		{
			std::cerr << "Image round-trip verification failed: " << filePath << '\n';
			return false;
		}

		return true;
	}
}

int main()
{
	const minicv::Image sourceImage = CreateSourceImage();
	const minicv::GrayscaleThresholdParameters thresholdParameters{ minicv::EThresholdType::BINARY, 100, 255 };
	const minicv::StructuringElement element = minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 3 });
	const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
	const minicv::Image binaryImage = minicv::CreateThresholdedGrayscaleImage(sourceImage, thresholdParameters);

	// Opening first prevents isolated bright noise from joining nearby objects during closing.
	const minicv::Image openedImage = minicv::CreateOpenedImage(binaryImage, element, borderParameters);
	const minicv::Image cleanedImage = minicv::CreateClosedImage(openedImage, element, borderParameters);
	const minicv::ConnectedComponents components(cleanedImage, minicv::EConnectivity::EIGHT);
	const minicv::Image candidateMask = CreateCandidateMask(components);

	const minicv::Image expectedCandidateMask = CreateExpectedCandidateMask();
	if (!HasExpectedComponents(components) || !candidateMask.HasSameContent(expectedCandidateMask))
	{
		std::cerr << "Unexpected object candidate detection result.\n";
		return 1;
	}

	std::error_code errorCode;
	const std::filesystem::path tempDirectory = std::filesystem::temp_directory_path(errorCode);
	if (errorCode)
	{
		std::cerr << "Failed to find a temporary directory.\n";
		return 1;
	}

	const std::filesystem::path outputDirectory = tempDirectory / "minicv_object_candidate_detection_example";
	std::filesystem::create_directories(outputDirectory, errorCode);
	if (errorCode)
	{
		std::cerr << "Failed to create output directory: " << outputDirectory << '\n';
		return 1;
	}

	if (!TrySaveAndVerifyImage(sourceImage, outputDirectory / "source.pgm") ||
		!TrySaveAndVerifyImage(binaryImage, outputDirectory / "binary.pgm") ||
		!TrySaveAndVerifyImage(openedImage, outputDirectory / "opened.pgm") ||
		!TrySaveAndVerifyImage(cleanedImage, outputDirectory / "cleaned.pgm") ||
		!TrySaveAndVerifyImage(candidateMask, outputDirectory / "candidates.pgm"))
	{
		return 1;
	}

	std::cout << "Object candidates (minimum area: " << MINIMUM_CANDIDATE_AREA << " pixels)\n";
	for (const minicv::ConnectedComponent& component : components.GetComponents())
	{
		if (component.Area < MINIMUM_CANDIDATE_AREA)
		{
			continue;
		}

		std::cout << "Label " << component.Label << ": area=" << component.Area
			<< ", bounds=(" << component.BoundingBox.X << ", " << component.BoundingBox.Y << ", " << component.BoundingBox.Width << ", " << component.BoundingBox.Height << ')'
			<< ", centroid=(" << component.CentroidX << ", " << component.CentroidY << ")\n";
	}

	std::cout << "Images: " << outputDirectory << '\n';
	return 0;
}
