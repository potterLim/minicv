#include <cassert>
#include <filesystem>
#include <fstream>
#include <optional>
#include <system_error>

#include "PgmImageIoTest.h"
#include "minicv/EImageType.h"
#include "minicv/EPgmFormat.h"
#include "minicv/Image.h"
#include "minicv/PgmImageIo.h"

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

	minicv::Image CreateSampleGrayscaleImage()
	{
		minicv::Image image(3, 2);

		image.GetGrayscalePixel(0, 0) = 0;
		image.GetGrayscalePixel(1, 0) = 128;
		image.GetGrayscalePixel(2, 0) = 255;
		image.GetGrayscalePixel(0, 1) = 10;
		image.GetGrayscalePixel(1, 1) = 20;
		image.GetGrayscalePixel(2, 1) = 30;

		return image;
	}

	void TestSaveAndLoadAsciiPgmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_ascii_test.pgm");
		RemoveFile(filePath);

		const minicv::Image image = CreateSampleGrayscaleImage();

		assert(minicv::TrySavePgmImage(image, filePath, minicv::EPgmFormat::ASCII));

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPgmImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->HasSameContent(image));

		RemoveFile(filePath);
	}

	void TestSaveAndLoadBinaryPgmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_binary_test.pgm");
		RemoveFile(filePath);

		const minicv::Image image = CreateSampleGrayscaleImage();

		assert(minicv::TrySavePgmImage(image, filePath, minicv::EPgmFormat::BINARY));

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPgmImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->HasSameContent(image));

		RemoveFile(filePath);
	}

	void TestLoadAsciiPgmImageWithComments()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_ascii_comment_test.pgm");
		RemoveFile(filePath);

		{
			std::ofstream outputStream(filePath, std::ios::binary);
			outputStream << "P2\n";
			outputStream << "# miniCV test image\n";
			outputStream << "3 2\n";
			outputStream << "255\n";
			outputStream << "0 128 255\n";
			outputStream << "10 # inline comment\n";
			outputStream << "20 30\n";
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPgmImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->GetWidth() == 3);
		assert(loadedImage->GetHeight() == 2);
		assert(loadedImage->GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
		assert(loadedImage->GetGrayscalePixel(0, 0) == 0);
		assert(loadedImage->GetGrayscalePixel(1, 0) == 128);
		assert(loadedImage->GetGrayscalePixel(2, 0) == 255);
		assert(loadedImage->GetGrayscalePixel(0, 1) == 10);
		assert(loadedImage->GetGrayscalePixel(1, 1) == 20);
		assert(loadedImage->GetGrayscalePixel(2, 1) == 30);

		RemoveFile(filePath);
	}

	void TestLoadBinaryPgmImageWithCrLfHeader()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_binary_crlf_test.pgm");
		RemoveFile(filePath);

		{
			std::ofstream outputStream(filePath, std::ios::binary);
			const char pixelData[] = {
				static_cast<char>(0),
				static_cast<char>(127),
				static_cast<char>(128),
				static_cast<char>(255)
			};

			outputStream << "P5\r\n";
			outputStream << "2 2\r\n";
			outputStream << "255\r\n";
			outputStream.write(pixelData, sizeof(pixelData));
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPgmImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->GetWidth() == 2);
		assert(loadedImage->GetHeight() == 2);
		assert(loadedImage->GetGrayscalePixel(0, 0) == 0);
		assert(loadedImage->GetGrayscalePixel(1, 0) == 127);
		assert(loadedImage->GetGrayscalePixel(0, 1) == 128);
		assert(loadedImage->GetGrayscalePixel(1, 1) == 255);

		RemoveFile(filePath);
	}

	void TestLoadMissingPgmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_missing_test.pgm");
		RemoveFile(filePath);

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPgmImage(filePath);

		assert(!loadedImage.has_value());
	}

	void TestLoadInvalidPgmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_invalid_test.pgm");
		RemoveFile(filePath);

		{
			std::ofstream outputStream(filePath, std::ios::binary);
			outputStream << "P2\n";
			outputStream << "2 1\n";
			outputStream << "255\n";
			outputStream << "0\n";
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPgmImage(filePath);

		assert(!loadedImage.has_value());

		RemoveFile(filePath);
	}

	void TestLoadOversizedPgmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_oversized_test.pgm");
		RemoveFile(filePath);

		{
			std::ofstream outputStream(filePath, std::ios::binary);
			outputStream << "P2\n";
			outputStream << "46341 46341\n";
			outputStream << "255\n";
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadPgmImage(filePath);

		assert(!loadedImage.has_value());

		RemoveFile(filePath);
	}

	void TestSaveRgbImageAsPgmFails()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_rgb_save_test.pgm");
		RemoveFile(filePath);

		const minicv::Image image(2, 2, minicv::EImageType::UINT8_RGB);

		assert(!minicv::TrySavePgmImage(image, filePath, minicv::EPgmFormat::ASCII));

		RemoveFile(filePath);
	}
}

void RunPgmImageIoTests()
{
	TestSaveAndLoadAsciiPgmImage();
	TestSaveAndLoadBinaryPgmImage();
	TestLoadAsciiPgmImageWithComments();
	TestLoadBinaryPgmImageWithCrLfHeader();
	TestLoadMissingPgmImage();
	TestLoadInvalidPgmImage();
	TestLoadOversizedPgmImage();
	TestSaveRgbImageAsPgmFails();
}
