#include <cassert>
#include <filesystem>
#include <fstream>
#include <optional>
#include <system_error>

#include "PpmImageIoTest.h"
#include "minicv/EImageType.h"
#include "minicv/EPpmFormat.h"
#include "minicv/ERgbChannel.h"
#include "minicv/Image.h"
#include "minicv/PpmImageIo.h"

namespace
{
	std::filesystem::path GetTestFilePath(const char* const fileName)
	{
		return std::filesystem::temp_directory_path() / fileName;
	}

	void RemoveFile(const std::filesystem::path& filePath)
	{
		std::error_code errorCode;
		std::filesystem::remove(filePath, errorCode);
	}

	minicv::Image CreateSampleRgbImage()
	{
		minicv::Image image(2, 2, minicv::EImageType::UINT8_RGB);

		image.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) = 255;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) = 0;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) = 0;

		image.GetRgbPixel(1, 0, minicv::ERgbChannel::RED) = 0;
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::GREEN) = 255;
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::BLUE) = 0;

		image.GetRgbPixel(0, 1, minicv::ERgbChannel::RED) = 0;
		image.GetRgbPixel(0, 1, minicv::ERgbChannel::GREEN) = 0;
		image.GetRgbPixel(0, 1, minicv::ERgbChannel::BLUE) = 255;

		image.GetRgbPixel(1, 1, minicv::ERgbChannel::RED) = 10;
		image.GetRgbPixel(1, 1, minicv::ERgbChannel::GREEN) = 20;
		image.GetRgbPixel(1, 1, minicv::ERgbChannel::BLUE) = 30;

		return image;
	}

	void TestSaveAndLoadAsciiPpmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_ascii_test.ppm");
		RemoveFile(filePath);

		const minicv::Image image = CreateSampleRgbImage();

		assert(minicv::TrySavePpmImage(image, filePath, minicv::EPpmFormat::ASCII));

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPpmImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->HasSameContent(image));

		RemoveFile(filePath);
	}

	void TestSaveAndLoadBinaryPpmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_binary_test.ppm");
		RemoveFile(filePath);

		const minicv::Image image = CreateSampleRgbImage();

		assert(minicv::TrySavePpmImage(image, filePath, minicv::EPpmFormat::BINARY));

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPpmImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->HasSameContent(image));

		RemoveFile(filePath);
	}

	void TestLoadAsciiPpmImageWithComments()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_ascii_comment_test.ppm");
		RemoveFile(filePath);

		{
			std::ofstream outputStream(filePath, std::ios::binary);
			outputStream << "P3\n";
			outputStream << "# miniCV RGB test image\n";
			outputStream << "2 1\n";
			outputStream << "255\n";
			outputStream << "255 0 0 # red pixel\n";
			outputStream << "0 255 0\n";
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPpmImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->GetWidth() == 2);
		assert(loadedImage->GetHeight() == 1);
		assert(loadedImage->GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(loadedImage->GetRgbPixel(0, 0, minicv::ERgbChannel::RED) == 255);
		assert(loadedImage->GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) == 0);
		assert(loadedImage->GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) == 0);
		assert(loadedImage->GetRgbPixel(1, 0, minicv::ERgbChannel::RED) == 0);
		assert(loadedImage->GetRgbPixel(1, 0, minicv::ERgbChannel::GREEN) == 255);
		assert(loadedImage->GetRgbPixel(1, 0, minicv::ERgbChannel::BLUE) == 0);

		RemoveFile(filePath);
	}

	void TestLoadBinaryPpmImageWithCrLfHeader()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_binary_crlf_test.ppm");
		RemoveFile(filePath);

		{
			std::ofstream outputStream(filePath, std::ios::binary);
			const char pixelData[] = {
				static_cast<char>(255),
				static_cast<char>(0),
				static_cast<char>(0),
				static_cast<char>(0),
				static_cast<char>(255),
				static_cast<char>(0)
			};

			outputStream << "P6\r\n";
			outputStream << "2 1\r\n";
			outputStream << "255\r\n";
			outputStream.write(pixelData, sizeof(pixelData));
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPpmImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->GetWidth() == 2);
		assert(loadedImage->GetHeight() == 1);
		assert(loadedImage->GetRgbPixel(0, 0, minicv::ERgbChannel::RED) == 255);
		assert(loadedImage->GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) == 0);
		assert(loadedImage->GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) == 0);
		assert(loadedImage->GetRgbPixel(1, 0, minicv::ERgbChannel::RED) == 0);
		assert(loadedImage->GetRgbPixel(1, 0, minicv::ERgbChannel::GREEN) == 255);
		assert(loadedImage->GetRgbPixel(1, 0, minicv::ERgbChannel::BLUE) == 0);

		RemoveFile(filePath);
	}

	void TestLoadMissingPpmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_missing_test.ppm");
		RemoveFile(filePath);

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPpmImage(filePath);

		assert(!loadedImage.has_value());
	}

	void TestLoadInvalidPpmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_invalid_test.ppm");
		RemoveFile(filePath);

		{
			std::ofstream outputStream(filePath, std::ios::binary);
			outputStream << "P3\n";
			outputStream << "1 1\n";
			outputStream << "255\n";
			outputStream << "255 0\n";
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPpmImage(filePath);

		assert(!loadedImage.has_value());

		RemoveFile(filePath);
	}

	void TestLoadOversizedPpmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_oversized_test.ppm");
		RemoveFile(filePath);

		{
			std::ofstream outputStream(filePath, std::ios::binary);
			outputStream << "P3\n";
			outputStream << "46341 46341\n";
			outputStream << "255\n";
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPpmImage(filePath);

		assert(!loadedImage.has_value());

		RemoveFile(filePath);
	}

	void TestSaveGrayscaleImageAsPpmFails()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_grayscale_save_test.ppm");
		RemoveFile(filePath);

		const minicv::Image image(2, 2);

		assert(!minicv::TrySavePpmImage(image, filePath, minicv::EPpmFormat::ASCII));

		RemoveFile(filePath);
	}
}

void RunPpmImageIoTests()
{
	TestSaveAndLoadAsciiPpmImage();
	TestSaveAndLoadBinaryPpmImage();
	TestLoadAsciiPpmImageWithComments();
	TestLoadBinaryPpmImageWithCrLfHeader();
	TestLoadMissingPpmImage();
	TestLoadInvalidPpmImage();
	TestLoadOversizedPpmImage();
	TestSaveGrayscaleImageAsPpmFails();
}
