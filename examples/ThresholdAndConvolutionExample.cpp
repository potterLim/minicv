#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <system_error>
#include <vector>

#include "minicv/ConvolutionKernel.h"
#include "minicv/EBorderType.h"
#include "minicv/EThresholdType.h"
#include "minicv/GrayscaleAdaptiveThresholdParameters.h"
#include "minicv/GrayscaleThresholdParameters.h"
#include "minicv/Image.h"
#include "minicv/ImageBorderParameters.h"
#include "minicv/ImageFiltering.h"
#include "minicv/ImageIo.h"
#include "minicv/ImageOperations.h"
#include "minicv/Rect.h"
#include "minicv/Size.h"

namespace
{
	constexpr int IMAGE_WIDTH = 12;
	constexpr int IMAGE_HEIGHT = 8;
	constexpr std::uint8_t BACKGROUND_VALUE = 30;
	constexpr std::uint8_t OBJECT_VALUE = 180;
	constexpr std::uint8_t TEXTURE_OFFSET = 20;
	constexpr minicv::Rect OBJECT_REGION{ 3, 2, 6, 4 };

	bool IsInsideRegion(const int x, const int y, const minicv::Rect region)
	{
		return x >= region.X && x < region.X + region.Width && y >= region.Y && y < region.Y + region.Height;
	}

	minicv::Image CreateSourceImage()
	{
		minicv::Image image(IMAGE_WIDTH, IMAGE_HEIGHT);

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				const std::uint8_t basePixelValue = IsInsideRegion(x, y, OBJECT_REGION) ? OBJECT_VALUE : BACKGROUND_VALUE;
				const std::uint8_t textureOffset = (x + y) % 2 == 0 ? 0 : TEXTURE_OFFSET;

				image.GetGrayscalePixel(x, y) = static_cast<std::uint8_t>(basePixelValue + textureOffset);
			}
		}

		return image;
	}

	bool HasMixedBinaryPixels(const minicv::Image& image)
	{
		const std::size_t nonZeroPixelCount = minicv::CountNonZeroPixels(image);
		return nonZeroPixelCount > 0 && nonZeroPixelCount < image.GetPixelCount();
	}

	bool TrySaveAndVerifyImage(const minicv::Image& image, const std::filesystem::path& filePath)
	{
		if (!minicv::TrySaveImage(image, filePath))
		{
			return false;
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);
		return loadedImage.has_value() && loadedImage->HasSameContent(image);
	}
}

int main()
{
	const minicv::Image sourceImage = CreateSourceImage();
	const std::vector<double> kernelCoefficients(9, 1.0 / 9.0);
	const minicv::ConvolutionKernel kernel(minicv::Size{ 3, 3 }, kernelCoefficients);
	const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
	const minicv::Image convolvedImage = minicv::CreateConvolvedImage(sourceImage, kernel, borderParameters);
	const std::optional<std::uint8_t> otsuThreshold = minicv::TryCalculateOtsuThreshold(convolvedImage);

	if (!otsuThreshold.has_value())
	{
		std::cerr << "Failed to calculate an Otsu threshold.\n";
		return 1;
	}

	const minicv::GrayscaleThresholdParameters otsuParameters{ minicv::EThresholdType::BINARY, *otsuThreshold, 255 };
	const minicv::GrayscaleAdaptiveThresholdParameters adaptiveParameters{ minicv::EThresholdType::BINARY, 255, 5, 0.0 };
	const minicv::Image otsuThresholdedImage = minicv::CreateThresholdedGrayscaleImage(convolvedImage, otsuParameters);
	const minicv::Image adaptiveThresholdedImage = minicv::CreateAdaptiveMeanThresholdedGrayscaleImage(convolvedImage, adaptiveParameters);

	if (convolvedImage.HasSameContent(sourceImage) || !HasMixedBinaryPixels(otsuThresholdedImage) || !HasMixedBinaryPixels(adaptiveThresholdedImage))
	{
		std::cerr << "Unexpected threshold or convolution result.\n";
		return 1;
	}

	std::error_code errorCode;
	const std::filesystem::path tempDirectory = std::filesystem::temp_directory_path(errorCode);
	if (errorCode)
	{
		std::cerr << "Failed to find a temporary directory.\n";
		return 1;
	}

	const std::filesystem::path outputDirectory = tempDirectory / "minicv_threshold_and_convolution_example";
	std::filesystem::create_directories(outputDirectory, errorCode);
	if (errorCode)
	{
		std::cerr << "Failed to create output directory: " << outputDirectory << '\n';
		return 1;
	}

	const std::filesystem::path sourceImagePath = outputDirectory / "source.pgm";
	const std::filesystem::path convolvedImagePath = outputDirectory / "convolved.pgm";
	const std::filesystem::path otsuThresholdedImagePath = outputDirectory / "otsu_thresholded.pgm";
	const std::filesystem::path adaptiveThresholdedImagePath = outputDirectory / "adaptive_thresholded.pgm";

	if (!TrySaveAndVerifyImage(sourceImage, sourceImagePath) ||
		!TrySaveAndVerifyImage(convolvedImage, convolvedImagePath) ||
		!TrySaveAndVerifyImage(otsuThresholdedImage, otsuThresholdedImagePath) ||
		!TrySaveAndVerifyImage(adaptiveThresholdedImage, adaptiveThresholdedImagePath))
	{
		std::cerr << "Failed to save and verify example images.\n";
		return 1;
	}

	std::cout << "Threshold and convolution example\n";
	std::cout << "Otsu threshold: " << static_cast<int>(*otsuThreshold) << '\n';
	std::cout << "Source image: " << sourceImagePath << '\n';
	std::cout << "Convolved image: " << convolvedImagePath << '\n';
	std::cout << "Otsu thresholded image: " << otsuThresholdedImagePath << '\n';
	std::cout << "Adaptive thresholded image: " << adaptiveThresholdedImagePath << '\n';

	return 0;
}
