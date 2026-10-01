#include <array>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <system_error>

#include "../src/NetpbmImageIo.h"
#include "ImageIoTest.h"
#include "minicv/EImageType.h"
#include "minicv/ERgbChannel.h"
#include "minicv/Image.h"
#include "minicv/ImageIo.h"

namespace
{
	class FailingFlushBuffer final : public std::stringbuf
	{
	protected:
		int sync() override
		{
			return -1;
		}
	};

	void TestBinaryWriteReportsFlushFailure()
	{
		FailingFlushBuffer buffer;
		std::ostream outputStream(&buffer);
		const minicv::Image image(1, 1);
		const bool isSaved = minicv::netpbm::TryWriteBinaryPixels(outputStream, image);
		assert(!isSaved && outputStream.fail());
		static_cast<void>(isSaved);
	}

	std::filesystem::path GetTestFilePath(const char* const fileName)
	{
		return std::filesystem::temp_directory_path() / fileName;
	}

	void RemoveFile(const std::filesystem::path& filePath)
	{
		std::error_code errorCode;
		std::filesystem::remove(filePath, errorCode);
	}

	void RemoveDirectory(const std::filesystem::path& directoryPath)
	{
		std::error_code errorCode;
		std::filesystem::remove_all(directoryPath, errorCode);
	}

	void WriteTextFile(const std::filesystem::path& filePath, const char* const text)
	{
		std::ofstream outputStream(filePath, std::ios::binary);
		outputStream << text;
	}

	void AssertSaveResult(const minicv::Image& image, const std::filesystem::path& filePath, const bool expectedResult)
	{
		const bool isSaved = minicv::TrySaveImage(image, filePath);
		assert(isSaved == expectedResult);
		static_cast<void>(isSaved);
		static_cast<void>(expectedResult);
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

		AssertSaveResult(image, filePath, true);

		const bool hasBinaryPgmMagicNumber = ReadMagicNumber(filePath) == "P5";
		assert(hasBinaryPgmMagicNumber);
		static_cast<void>(hasBinaryPgmMagicNumber);

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

		AssertSaveResult(image, filePath, true);

		const bool hasBinaryPpmMagicNumber = ReadMagicNumber(filePath) == "P6";
		assert(hasBinaryPpmMagicNumber);
		static_cast<void>(hasBinaryPpmMagicNumber);

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

		AssertSaveResult(grayscaleImage, pgmFilePath, true);
		AssertSaveResult(rgbImage, ppmFilePath, true);

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
		AssertSaveResult(CreateSampleGrayscaleImage(), filePath, false);

		RemoveFile(filePath);
	}

	void TestMismatchedSaveExtensionFails()
	{
		const std::filesystem::path pgmFilePath = GetTestFilePath("minicv_rgb_as_pgm_test.pgm");
		const std::filesystem::path ppmFilePath = GetTestFilePath("minicv_grayscale_as_ppm_test.ppm");
		RemoveFile(pgmFilePath);
		RemoveFile(ppmFilePath);

		AssertSaveResult(CreateSampleRgbImage(), pgmFilePath, false);
		AssertSaveResult(CreateSampleGrayscaleImage(), ppmFilePath, false);

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

	void TestMissingInputFileFails()
	{
		const std::filesystem::path filePath = GetTestFilePath("minicv_missing_input_file.pgm");
		RemoveFile(filePath);

		assert(!minicv::TryLoadImage(filePath).has_value());
	}

	void TestMissingOutputDirectoryFails()
	{
		const std::filesystem::path directoryPath = GetTestFilePath("minicv_missing_output_directory");
		const std::filesystem::path filePath = directoryPath / "image.pgm";
		RemoveDirectory(directoryPath);

		AssertSaveResult(CreateSampleGrayscaleImage(), filePath, false);
		assert(!std::filesystem::exists(filePath));
	}

	void TestInvalidHeaderValuesFail()
	{
		const std::filesystem::path badMagicNumberPath = GetTestFilePath("minicv_bad_magic_number.pgm");
		const std::filesystem::path invalidWidthPath = GetTestFilePath("minicv_invalid_width.pgm");
		const std::filesystem::path zeroHeightPath = GetTestFilePath("minicv_zero_height.pgm");
		const std::filesystem::path unsupportedMaxValuePath = GetTestFilePath("minicv_unsupported_max_value.pgm");
		RemoveFile(badMagicNumberPath);
		RemoveFile(invalidWidthPath);
		RemoveFile(zeroHeightPath);
		RemoveFile(unsupportedMaxValuePath);

		WriteTextFile(badMagicNumberPath, "P1\n1 1\n255\n0\n");
		WriteTextFile(invalidWidthPath, "P2\nabc 1\n255\n0\n");
		WriteTextFile(zeroHeightPath, "P2\n1 0\n255\n0\n");
		WriteTextFile(unsupportedMaxValuePath, "P2\n1 1\n1023\n0\n");

		assert(!minicv::TryLoadImage(badMagicNumberPath).has_value());
		assert(!minicv::TryLoadImage(invalidWidthPath).has_value());
		assert(!minicv::TryLoadImage(zeroHeightPath).has_value());
		assert(!minicv::TryLoadImage(unsupportedMaxValuePath).has_value());

		RemoveFile(badMagicNumberPath);
		RemoveFile(invalidWidthPath);
		RemoveFile(zeroHeightPath);
		RemoveFile(unsupportedMaxValuePath);
	}

	void TestInvalidAsciiPixelValuesFail()
	{
		const std::filesystem::path negativePixelPath = GetTestFilePath("minicv_negative_pixel.pgm");
		const std::filesystem::path largePixelPath = GetTestFilePath("minicv_large_pixel.pgm");
		const std::filesystem::path nonNumericPixelPath = GetTestFilePath("minicv_non_numeric_pixel.ppm");
		RemoveFile(negativePixelPath);
		RemoveFile(largePixelPath);
		RemoveFile(nonNumericPixelPath);

		WriteTextFile(negativePixelPath, "P2\n1 1\n255\n-1\n");
		WriteTextFile(largePixelPath, "P2\n1 1\n255\n256\n");
		WriteTextFile(nonNumericPixelPath, "P3\n1 1\n255\n255 green 0\n");

		assert(!minicv::TryLoadImage(negativePixelPath).has_value());
		assert(!minicv::TryLoadImage(largePixelPath).has_value());
		assert(!minicv::TryLoadImage(nonNumericPixelPath).has_value());

		RemoveFile(negativePixelPath);
		RemoveFile(largePixelPath);
		RemoveFile(nonNumericPixelPath);
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

	void TestBinarySeparatorsPreserveRasterBytes()
	{
		const std::array<std::string, 4> separators{ "\r", "\n", " ", "\r\n" };
		for (const std::string& separator : separators)
		{
			for (const bool isRgb : { false, true })
			{
				for (const int trailingByteCount : { 0, 1, 4 })
				{
					const std::filesystem::path filePath = GetTestFilePath(isRgb ? "minicv_separator_review.ppm" : "minicv_separator_review.pgm");
					const std::array<unsigned char, 6> pixels{ 10, 13, 32, 35, 0, 255 };
					{
						std::ofstream outputStream(filePath, std::ios::binary);
						const char* const lineEnding = separator == "\r\n" ? "\r\n" : "\n";
						outputStream << (isRgb ? "P6" : "P5") << lineEnding;
						outputStream << (isRgb ? "2 1" : "6 1") << lineEnding << "255" << separator;
						outputStream.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
						for (int index = 0; index < trailingByteCount; ++index)
						{
							outputStream.put('\n');
						}
					}
					const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);
					assert(loadedImage.has_value());
					for (std::size_t index = 0; index < pixels.size(); ++index)
					{
						const bool isPixelEqual = loadedImage->GetPixelData()[index] == pixels[index];
						assert(isPixelEqual);
						static_cast<void>(isPixelEqual);
					}
					RemoveFile(filePath);
				}
			}
		}
	}

	void TestBinaryCrLfWithTrailingAndTruncatedData()
	{
		for (const bool isRgb : { false, true })
		{
			const std::filesystem::path filePath = GetTestFilePath(isRgb ? "minicv_crlf_regression.ppm" : "minicv_crlf_regression.pgm");
			const std::array<unsigned char, 6> pixels{ 10, 13, 32, 35, 0, 255 };
			for (const int byteAdjustment : { -1, 0, 1, 4 })
			{
				{
					std::ofstream outputStream(filePath, std::ios::binary);
					outputStream << (isRgb ? "P6\r\n2 1\r\n255\r\n" : "P5\r\n6 1\r\n255\r\n");
					const std::streamsize byteCount = byteAdjustment < 0 ? 5 : 6;
					outputStream.write(reinterpret_cast<const char*>(pixels.data()), byteCount);
					for (int index = 0; index < byteAdjustment; ++index)
					{
						outputStream.put('\n');
					}
				}
				const std::optional<minicv::Image> loadedImage = minicv::TryLoadImage(filePath);
				assert(loadedImage.has_value() == (byteAdjustment >= 0));
				if (loadedImage.has_value())
				{
					for (std::size_t index = 0; index < pixels.size(); ++index)
					{
						assert(loadedImage->GetPixelData()[index] == pixels[index]);
					}
				}
			}
			RemoveFile(filePath);
		}
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
	TestBinaryWriteReportsFlushFailure();
	TestBinarySeparatorsPreserveRasterBytes();
	TestBinaryCrLfWithTrailingAndTruncatedData();
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
	TestMissingInputFileFails();
	TestMissingOutputDirectoryFails();
	TestInvalidHeaderValuesFail();
	TestInvalidAsciiPixelValuesFail();
	TestInvalidImageFilesFail();
	TestOversizedImageFilesFail();
}
