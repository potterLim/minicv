#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <system_error>

#include "minicv/GrayscaleCumulativeDistribution.h"
#include "minicv/GrayscaleHistogram.h"
#include "minicv/GrayscaleValueRange.h"
#include "minicv/Image.h"
#include "minicv/ImageIo.h"
#include "minicv/ImageOperations.h"

namespace
{
	constexpr int IMAGE_WIDTH = 8;
	constexpr int IMAGE_HEIGHT = 4;
	constexpr int BASE_PIXEL_VALUE = 72;
	constexpr int X_PIXEL_STEP = 8;
	constexpr int Y_PIXEL_STEP = 4;
	constexpr int EXPECTED_MINIMUM_PIXEL_VALUE = BASE_PIXEL_VALUE;
	constexpr int EXPECTED_MAXIMUM_PIXEL_VALUE = BASE_PIXEL_VALUE + (IMAGE_WIDTH - 1) * X_PIXEL_STEP + (IMAGE_HEIGHT - 1) * Y_PIXEL_STEP;

	minicv::Image CreateLowContrastImage()
	{
		minicv::Image image(IMAGE_WIDTH, IMAGE_HEIGHT);

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				const int pixelValue = BASE_PIXEL_VALUE + x * X_PIXEL_STEP + y * Y_PIXEL_STEP;
				image.GetGrayscalePixel(x, y) = static_cast<std::uint8_t>(pixelValue);
			}
		}

		return image;
	}

	std::size_t GetHistogramPixelCount(const minicv::GrayscaleHistogram& histogram)
	{
		std::size_t pixelCount = 0;

		for (const std::size_t binCount : histogram.BinCounts)
		{
			pixelCount += binCount;
		}

		return pixelCount;
	}

	void PrintNonZeroHistogramBins(const minicv::GrayscaleHistogram& histogram)
	{
		for (std::size_t binIndex = 0; binIndex < histogram.BinCounts.size(); ++binIndex)
		{
			const std::size_t binCount = histogram.BinCounts[binIndex];

			if (binCount != 0)
			{
				std::cout << "  " << binIndex << ": " << binCount << '\n';
			}
		}
	}
}

int main()
{
	const minicv::Image sourceImage = CreateLowContrastImage();
	const minicv::GrayscaleHistogram sourceHistogram = minicv::CalculateGrayscaleHistogram(sourceImage);
	const minicv::GrayscaleCumulativeDistribution sourceCumulativeDistribution = minicv::CalculateGrayscaleCumulativeDistribution(sourceHistogram);
	const std::optional<minicv::GrayscaleValueRange> sourceValueRange = minicv::TryGetGrayscaleValueRange(sourceImage);

	const minicv::Image normalizedImage = minicv::CreateMinMaxNormalizedGrayscaleImage(sourceImage);
	const minicv::GrayscaleHistogram normalizedHistogram = minicv::CalculateGrayscaleHistogram(normalizedImage);
	const std::optional<minicv::GrayscaleValueRange> normalizedValueRange = minicv::TryGetGrayscaleValueRange(normalizedImage);

	if (!sourceValueRange.has_value() || sourceValueRange->Minimum != EXPECTED_MINIMUM_PIXEL_VALUE ||
		sourceValueRange->Maximum != EXPECTED_MAXIMUM_PIXEL_VALUE)
	{
		std::cerr << "Unexpected source image value range.\n";
		return 1;
	}

	if (!normalizedValueRange.has_value() || normalizedValueRange->Minimum != 0 || normalizedValueRange->Maximum != 255)
	{
		std::cerr << "Unexpected normalized image value range.\n";
		return 1;
	}

	if (GetHistogramPixelCount(sourceHistogram) != sourceImage.GetPixelCount() ||
		GetHistogramPixelCount(normalizedHistogram) != normalizedImage.GetPixelCount() ||
		sourceCumulativeDistribution.Values.back() != 1.0)
	{
		std::cerr << "Unexpected histogram result.\n";
		return 1;
	}

	std::error_code errorCode;
	const std::filesystem::path tempDirectory = std::filesystem::temp_directory_path(errorCode);
	if (errorCode)
	{
		std::cerr << "Failed to find a temporary directory.\n";
		return 1;
	}

	const std::filesystem::path outputDirectory = tempDirectory / "minicv_histogram_example";
	std::filesystem::create_directories(outputDirectory, errorCode);
	if (errorCode)
	{
		std::cerr << "Failed to create output directory: " << outputDirectory << '\n';
		return 1;
	}

	const std::filesystem::path sourceImagePath = outputDirectory / "source.pgm";
	const std::filesystem::path normalizedImagePath = outputDirectory / "normalized.pgm";

	if (!minicv::TrySaveImage(sourceImage, sourceImagePath) || !minicv::TrySaveImage(normalizedImage, normalizedImagePath))
	{
		std::cerr << "Failed to save histogram example images.\n";
		return 1;
	}

	std::cout << "Histogram and normalization example\n";
	std::cout << "Source value range: " << static_cast<int>(sourceValueRange->Minimum) << '-' << static_cast<int>(sourceValueRange->Maximum) << '\n';
	std::cout << "Source CDF at minimum: " << sourceCumulativeDistribution.Values[static_cast<std::size_t>(sourceValueRange->Minimum)] << '\n';
	std::cout << "Source histogram:\n";
	PrintNonZeroHistogramBins(sourceHistogram);
	std::cout << "Normalized histogram:\n";
	PrintNonZeroHistogramBins(normalizedHistogram);
	std::cout << "Source image: " << sourceImagePath << '\n';
	std::cout << "Normalized image: " << normalizedImagePath << '\n';

	return 0;
}
