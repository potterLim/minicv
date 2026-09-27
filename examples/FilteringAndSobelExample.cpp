#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <system_error>

#include "minicv/EBorderType.h"
#include "minicv/GrayscaleFilterResponse.h"
#include "minicv/Image.h"
#include "minicv/ImageBorderParameters.h"
#include "minicv/ImageFiltering.h"
#include "minicv/ImageIo.h"
#include "minicv/ImageOperations.h"
#include "minicv/Point.h"
#include "minicv/Rect.h"
#include "minicv/Size.h"

namespace
{
	constexpr int IMAGE_WIDTH = 32;
	constexpr int IMAGE_HEIGHT = 24;
	constexpr std::uint8_t BACKGROUND_VALUE = 30;
	constexpr std::uint8_t OBJECT_VALUE = 180;
	constexpr minicv::Rect OBJECT_REGION{ 0, 6, 20, 12 };
	constexpr minicv::Point BRIGHT_NOISE_POSITION{ 25, 4 };
	constexpr minicv::Point DARK_NOISE_POSITION{ 10, 12 };
	constexpr minicv::Size KERNEL_SIZE{ 3, 3 };
	constexpr double GAUSSIAN_STANDARD_DEVIATION = 1.0;

	[[nodiscard]] minicv::Image CreateSourceImage()
	{
		minicv::Image image(IMAGE_WIDTH, IMAGE_HEIGHT);
		image.Fill(BACKGROUND_VALUE);

		// Touch the left boundary so the border modes also differ along the object.
		for (int y = OBJECT_REGION.Y; y < OBJECT_REGION.Y + OBJECT_REGION.Height; ++y)
		{
			for (int x = OBJECT_REGION.X; x < OBJECT_REGION.X + OBJECT_REGION.Width; ++x)
			{
				image.GetGrayscalePixel(x, y) = OBJECT_VALUE;
			}
		}

		image.GetGrayscalePixel(BRIGHT_NOISE_POSITION.X, BRIGHT_NOISE_POSITION.Y) = 255;
		image.GetGrayscalePixel(DARK_NOISE_POSITION.X, DARK_NOISE_POSITION.Y) = 0;
		return image;
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

	[[nodiscard]] bool TrySaveFilteringComparison(
		const minicv::Image& sourceImage,
		const minicv::ImageBorderParameters borderParameters,
		const std::filesystem::path& outputDirectory)
	{
		std::error_code errorCode;
		std::filesystem::create_directories(outputDirectory, errorCode);
		if (errorCode)
		{
			std::cerr << "Failed to create output directory: " << outputDirectory << '\n';
			return false;
		}

		const minicv::Image boxBlurredImage = minicv::CreateBoxBlurredImage(sourceImage, KERNEL_SIZE, borderParameters);
		const minicv::Image gaussianBlurredImage = minicv::CreateGaussianBlurredImage(
			sourceImage, KERNEL_SIZE, GAUSSIAN_STANDARD_DEVIATION, borderParameters);
		const minicv::Image medianFilteredImage = minicv::CreateMedianFilteredImage(sourceImage, KERNEL_SIZE, borderParameters);
		const minicv::Image sharpenedImage = minicv::CreateSharpenedImage(sourceImage, borderParameters);
		const minicv::GrayscaleFilterResponse laplacianResponse = minicv::CreateLaplacianResponse(sourceImage, borderParameters);
		const minicv::Image laplacianImage = minicv::CreateSignedResponseImage(laplacianResponse);

		const minicv::GrayscaleFilterResponse sobelXResponse = minicv::CreateSobelXResponse(sourceImage, borderParameters);
		const minicv::GrayscaleFilterResponse sobelYResponse = minicv::CreateSobelYResponse(sourceImage, borderParameters);
		const minicv::Image sobelXImage = minicv::CreateSignedResponseImage(sobelXResponse);
		const minicv::Image sobelYImage = minicv::CreateSignedResponseImage(sobelYResponse);
		const minicv::GrayscaleFilterResponse magnitudeResponse = minicv::CreateGradientMagnitudeResponse(sobelXResponse, sobelYResponse);
		const minicv::Image edgeImage = minicv::CreateNormalizedGradientMagnitudeImage(magnitudeResponse);

		const minicv::GrayscaleFilterResponse blurredSobelXResponse = minicv::CreateSobelXResponse(gaussianBlurredImage, borderParameters);
		const minicv::GrayscaleFilterResponse blurredSobelYResponse = minicv::CreateSobelYResponse(gaussianBlurredImage, borderParameters);
		const minicv::GrayscaleFilterResponse blurredMagnitudeResponse = minicv::CreateGradientMagnitudeResponse(
			blurredSobelXResponse, blurredSobelYResponse);
		const minicv::Image blurredEdgeImage = minicv::CreateNormalizedGradientMagnitudeImage(blurredMagnitudeResponse);

		if (boxBlurredImage.HasSameContent(sourceImage) || gaussianBlurredImage.HasSameContent(sourceImage) ||
			medianFilteredImage.GetGrayscalePixel(BRIGHT_NOISE_POSITION.X, BRIGHT_NOISE_POSITION.Y) != BACKGROUND_VALUE ||
			medianFilteredImage.GetGrayscalePixel(DARK_NOISE_POSITION.X, DARK_NOISE_POSITION.Y) != OBJECT_VALUE ||
			minicv::CountNonZeroPixels(edgeImage) == 0 || edgeImage.HasSameContent(blurredEdgeImage))
		{
			std::cerr << "Unexpected filtering or Sobel result.\n";
			return false;
		}

		return TrySaveAndVerifyImage(sourceImage, outputDirectory / "source.pgm") &&
			TrySaveAndVerifyImage(boxBlurredImage, outputDirectory / "box_blurred.pgm") &&
			TrySaveAndVerifyImage(gaussianBlurredImage, outputDirectory / "gaussian_blurred.pgm") &&
			TrySaveAndVerifyImage(medianFilteredImage, outputDirectory / "median_filtered.pgm") &&
			TrySaveAndVerifyImage(sharpenedImage, outputDirectory / "sharpened.pgm") &&
			TrySaveAndVerifyImage(laplacianImage, outputDirectory / "laplacian_signed.pgm") &&
			TrySaveAndVerifyImage(sobelXImage, outputDirectory / "sobel_x_signed.pgm") &&
			TrySaveAndVerifyImage(sobelYImage, outputDirectory / "sobel_y_signed.pgm") &&
			TrySaveAndVerifyImage(edgeImage, outputDirectory / "sobel_magnitude.pgm") &&
			TrySaveAndVerifyImage(blurredEdgeImage, outputDirectory / "gaussian_sobel_magnitude.pgm");
	}
}

int main()
{
	std::error_code errorCode;
	const std::filesystem::path tempDirectory = std::filesystem::temp_directory_path(errorCode);
	if (errorCode)
	{
		std::cerr << "Failed to find a temporary directory.\n";
		return 1;
	}

	const std::filesystem::path outputDirectory = tempDirectory / "minicv_filtering_and_sobel_example";
	const minicv::Image sourceImage = CreateSourceImage();
	const minicv::ImageBorderParameters constantBorderParameters{ minicv::EBorderType::CONSTANT, 0 };
	const minicv::ImageBorderParameters replicateBorderParameters{ minicv::EBorderType::REPLICATE, 0 };

	if (!TrySaveFilteringComparison(sourceImage, constantBorderParameters, outputDirectory / "constant") ||
		!TrySaveFilteringComparison(sourceImage, replicateBorderParameters, outputDirectory / "replicate"))
	{
		return 1;
	}

	std::cout << "Filtering and Sobel comparison: " << outputDirectory << '\n';
	std::cout << "Compare constant/ and replicate/ for border effects.\n";
	std::cout << "Signed responses: zero is gray (128), negative is darker, positive is brighter.\n";
	std::cout << "Magnitude images are normalized independently; compare edge locations and shapes.\n";
	return 0;
}
