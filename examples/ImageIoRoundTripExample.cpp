#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <system_error>

#include "minicv/EImageType.h"
#include "minicv/ERgbChannel.h"
#include "minicv/Image.h"
#include "minicv/ImageIo.h"

namespace
{
	constexpr int GRAYSCALE_IMAGE_WIDTH = 4;
	constexpr int GRAYSCALE_IMAGE_HEIGHT = 3;
	constexpr int RGB_IMAGE_WIDTH = 3;
	constexpr int RGB_IMAGE_HEIGHT = 2;
	constexpr int GRAYSCALE_X_STEP = 40;
	constexpr int GRAYSCALE_Y_STEP = 50;
	constexpr int RED_X_STEP = 80;
	constexpr int GREEN_Y_STEP = 120;
	constexpr int BLUE_BASE_VALUE = 50;
	constexpr int BLUE_X_STEP = 20;
	constexpr int BLUE_Y_STEP = 30;

	minicv::Image CreateGrayscaleImage()
	{
		minicv::Image image(GRAYSCALE_IMAGE_WIDTH, GRAYSCALE_IMAGE_HEIGHT);

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				const int pixelValue = x * GRAYSCALE_X_STEP + y * GRAYSCALE_Y_STEP;
				image.GetGrayscalePixel(x, y) = static_cast<std::uint8_t>(pixelValue);
			}
		}

		return image;
	}

	minicv::Image CreateRgbImage()
	{
		minicv::Image image(RGB_IMAGE_WIDTH, RGB_IMAGE_HEIGHT, minicv::EImageType::UINT8_RGB);

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				const int blueValue = BLUE_BASE_VALUE + x * BLUE_X_STEP + y * BLUE_Y_STEP;

				image.GetRgbPixel(x, y, minicv::ERgbChannel::RED) = static_cast<std::uint8_t>(x * RED_X_STEP);
				image.GetRgbPixel(x, y, minicv::ERgbChannel::GREEN) = static_cast<std::uint8_t>(y * GREEN_Y_STEP);
				image.GetRgbPixel(x, y, minicv::ERgbChannel::BLUE) = static_cast<std::uint8_t>(blueValue);
			}
		}

		return image;
	}

	bool TryRoundTripImage(const minicv::Image& image, const std::filesystem::path& filePath)
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
	std::error_code errorCode;
	const std::filesystem::path tempDirectory = std::filesystem::temp_directory_path(errorCode);
	if (errorCode)
	{
		std::cerr << "Failed to find a temporary directory.\n";
		return 1;
	}

	const std::filesystem::path outputDirectory = tempDirectory / "minicv_image_io_round_trip_example";
	std::filesystem::create_directories(outputDirectory, errorCode);
	if (errorCode)
	{
		std::cerr << "Failed to create output directory: " << outputDirectory << '\n';
		return 1;
	}

	const minicv::Image grayscaleImage = CreateGrayscaleImage();
	const minicv::Image rgbImage = CreateRgbImage();

	const std::filesystem::path grayscaleFilePath = outputDirectory / "round_trip_grayscale.pgm";
	const std::filesystem::path rgbFilePath = outputDirectory / "round_trip_rgb.ppm";

	if (!TryRoundTripImage(grayscaleImage, grayscaleFilePath))
	{
		std::cerr << "Failed to round-trip grayscale image.\n";
		return 1;
	}

	if (!TryRoundTripImage(rgbImage, rgbFilePath))
	{
		std::cerr << "Failed to round-trip RGB image.\n";
		return 1;
	}

	std::cout << "Image I/O round-trip example\n";
	std::cout << "Grayscale image: " << grayscaleFilePath << '\n';
	std::cout << "RGB image: " << rgbFilePath << '\n';

	return 0;
}
