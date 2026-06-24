#include <cassert>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <system_error>

#include "ImageIoTest.h"
#include "minicv/EImageType.h"
#include "minicv/ERgbChannel.h"
#include "minicv/Image.h"
#include "minicv/ImageIo.h"

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

	std::string ReadMagicNumber(const std::filesystem::path& filePath)
	{
		std::ifstream inputStream(filePath, std::ios::binary);

		std::string magicNumber;
		inputStream >> magicNumber;

		return magicNumber;
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

	void TestSaveAndLoadPgmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_binary_test.pgm");
		RemoveFile(filePath);

		const minicv::Image image = CreateSampleGrayscaleImage();

		assert(minicv::TrySaveImage(image, filePath));

		const bool hasBinaryPgmMagicNumber = ReadMagicNumber(filePath) == "P5";
		assert(hasBinaryPgmMagicNumber);
		(void)hasBinaryPgmMagicNumber;

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->HasSameContent(image));

		RemoveFile(filePath);
	}

	void TestSaveAndLoadPpmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_binary_test.ppm");
		RemoveFile(filePath);

		const minicv::Image image = CreateSampleRgbImage();

		assert(minicv::TrySaveImage(image, filePath));

		const bool hasBinaryPpmMagicNumber = ReadMagicNumber(filePath) == "P6";
		assert(hasBinaryPpmMagicNumber);
		(void)hasBinaryPpmMagicNumber;

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->HasSameContent(image));

		RemoveFile(filePath);
	}

	void TestLoadAsciiPgmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_ascii_test.pgm");
		RemoveFile(filePath);

		{
			std::ofstream outputStream(filePath, std::ios::binary);
			outputStream << "P2\n";
			outputStream << "# miniCV grayscale test image\n";
			outputStream << "3 2\n";
			outputStream << "255\n";
			outputStream << "0 128 255\n";
			outputStream << "10 # inline comment\n";
			outputStream << "20 30\n";
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);
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

	void TestLoadAsciiPpmImage()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_ascii_test.ppm");
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

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);
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

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);
		assert(loadedImage.has_value());
		assert(loadedImage->GetWidth() == 2);
		assert(loadedImage->GetHeight() == 2);
		assert(loadedImage->GetGrayscalePixel(0, 0) == 0);
		assert(loadedImage->GetGrayscalePixel(1, 0) == 127);
		assert(loadedImage->GetGrayscalePixel(0, 1) == 128);
		assert(loadedImage->GetGrayscalePixel(1, 1) == 255);

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

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);
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

	void TestUppercaseExtensions()
	{
		const std::filesystem::path pgmFilePath = GetTestFilePath("minicv_uppercase_test.PGM");
		const std::filesystem::path ppmFilePath = GetTestFilePath("minicv_uppercase_test.PPM");
		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);

		const minicv::Image grayscaleImage = CreateSampleGrayscaleImage();
		const minicv::Image rgbImage = CreateSampleRgbImage();

		assert(minicv::TrySaveImage(grayscaleImage, pgmFilePath));
		assert(minicv::TrySaveImage(rgbImage, ppmFilePath));

		const std::optional<minicv::Image> loadedGrayscaleImage = minicv::TryLoadImage(pgmFilePath);
		const std::optional<minicv::Image> loadedRgbImage = minicv::TryLoadImage(ppmFilePath);

		assert(loadedGrayscaleImage.has_value());
		assert(loadedRgbImage.has_value());
		assert(loadedGrayscaleImage->HasSameContent(grayscaleImage));
		assert(loadedRgbImage->HasSameContent(rgbImage));

		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);
	}

	void TestUnknownExtensionFails()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_unknown_extension_test.txt");
		RemoveFile(filePath);

		{
			std::ofstream outputStream(filePath, std::ios::binary);
			outputStream << "P2\n";
			outputStream << "1 1\n";
			outputStream << "255\n";
			outputStream << "0\n";
		}

		const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);

		assert(!loadedImage.has_value());
		assert(!minicv::TrySaveImage(CreateSampleGrayscaleImage(), filePath));

		RemoveFile(filePath);
	}

	void TestMismatchedSaveExtensionFails()
	{
		const std::filesystem::path pgmFilePath = GetTestFilePath("minicv_rgb_as_pgm_test.pgm");
		const std::filesystem::path ppmFilePath = GetTestFilePath("minicv_grayscale_as_ppm_test.ppm");
		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);

		assert(!minicv::TrySaveImage(CreateSampleRgbImage(), pgmFilePath));
		assert(!minicv::TrySaveImage(CreateSampleGrayscaleImage(), ppmFilePath));

		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);
	}

	void TestMismatchedLoadExtensionFails()
	{
		const std::filesystem::path pgmFilePath = GetTestFilePath("minicv_ppm_as_pgm_test.pgm");
		const std::filesystem::path ppmFilePath = GetTestFilePath("minicv_pgm_as_ppm_test.ppm");
		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);

		{
			std::ofstream outputStream(pgmFilePath, std::ios::binary);
			outputStream << "P3\n";
			outputStream << "1 1\n";
			outputStream << "255\n";
			outputStream << "255 0 0\n";
		}

		{
			std::ofstream outputStream(ppmFilePath, std::ios::binary);
			outputStream << "P2\n";
			outputStream << "1 1\n";
			outputStream << "255\n";
			outputStream << "128\n";
		}

		assert(!minicv::TryLoadImage(pgmFilePath).has_value());
		assert(!minicv::TryLoadImage(ppmFilePath).has_value());

		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);
	}

	void TestInvalidImageFilesFail()
	{
		const std::filesystem::path pgmFilePath = GetTestFilePath("minicv_invalid_test.pgm");
		const std::filesystem::path ppmFilePath = GetTestFilePath("minicv_invalid_test.ppm");
		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);

		{
			std::ofstream outputStream(pgmFilePath, std::ios::binary);
			outputStream << "P2\n";
			outputStream << "2 1\n";
			outputStream << "255\n";
			outputStream << "0\n";
		}

		{
			std::ofstream outputStream(ppmFilePath, std::ios::binary);
			outputStream << "P3\n";
			outputStream << "1 1\n";
			outputStream << "255\n";
			outputStream << "255 0\n";
		}

		assert(!minicv::TryLoadImage(pgmFilePath).has_value());
		assert(!minicv::TryLoadImage(ppmFilePath).has_value());

		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);
	}

	void TestOversizedImageFilesFail()
	{
		const std::filesystem::path pgmFilePath = GetTestFilePath("minicv_oversized_test.pgm");
		const std::filesystem::path ppmFilePath = GetTestFilePath("minicv_oversized_test.ppm");
		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);

		{
			std::ofstream outputStream(pgmFilePath, std::ios::binary);
			outputStream << "P2\n";
			outputStream << "46341 46341\n";
			outputStream << "255\n";
		}

		{
			std::ofstream outputStream(ppmFilePath, std::ios::binary);
			outputStream << "P3\n";
			outputStream << "46341 46341\n";
			outputStream << "255\n";
		}

		assert(!minicv::TryLoadImage(pgmFilePath).has_value());
		assert(!minicv::TryLoadImage(ppmFilePath).has_value());

		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);
	}
}

void RunImageIoTests()
{
	TestSaveAndLoadPgmImage();
	TestSaveAndLoadPpmImage();
	TestLoadAsciiPgmImage();
	TestLoadAsciiPpmImage();
	TestLoadBinaryPgmImageWithCrLfHeader();
	TestLoadBinaryPpmImageWithCrLfHeader();
	TestUppercaseExtensions();
	TestUnknownExtensionFails();
	TestMismatchedSaveExtensionFails();
	TestMismatchedLoadExtensionFails();
	TestInvalidImageFilesFail();
	TestOversizedImageFilesFail();
}
