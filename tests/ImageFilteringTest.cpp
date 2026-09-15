#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "ImageFilteringTest.h"
#include "minicv/ConvolutionKernel.h"
#include "minicv/GrayscaleFilterResponse.h"
#include "minicv/Image.h"
#include "minicv/ImageFiltering.h"

namespace
{
	constexpr double COMPARISON_TOLERANCE = 1e-12;

	template <std::size_t PIXEL_COUNT>
	void SetGrayscalePixels(minicv::Image& image, const std::array<std::uint8_t, PIXEL_COUNT>& pixelValues)
	{
		const bool doesPixelCountMatch = image.GetPixelCount() == PIXEL_COUNT;
		assert(doesPixelCountMatch);

		(void)doesPixelCountMatch;

		for (std::size_t pixelIndex = 0; pixelIndex < PIXEL_COUNT; ++pixelIndex)
		{
			image.GetPixelData()[pixelIndex] = pixelValues[pixelIndex];
		}
	}

	template <std::size_t PIXEL_COUNT>
	void AssertGrayscalePixelsEqual(const minicv::Image& image, const std::array<std::uint8_t, PIXEL_COUNT>& expectedPixelValues)
	{
		const bool doesPixelCountMatch = image.GetPixelCount() == PIXEL_COUNT;
		assert(doesPixelCountMatch);

		(void)doesPixelCountMatch;

		for (std::size_t pixelIndex = 0; pixelIndex < PIXEL_COUNT; ++pixelIndex)
		{
			const bool isPixelValueEqual = image.GetPixelData()[pixelIndex] == expectedPixelValues[pixelIndex];
			assert(isPixelValueEqual);

			(void)isPixelValueEqual;
		}
	}

	void TestIdentityConvolutionPreservesImage()
	{
		minicv::Image image(2, 2);
		SetGrayscalePixels(image, std::array<std::uint8_t, 4>{ 10, 20, 30, 40 });

		const minicv::ConvolutionKernel kernel(minicv::Size{ 1, 1 }, std::vector<double>{ 1.0 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 0 };
		const minicv::Image convolvedImage = minicv::CreateConvolvedImage(image, kernel, borderParameters);
		const bool isContentEqual = convolvedImage.HasSameContent(image);

		assert(isContentEqual);

		(void)isContentEqual;
	}

	void TestConvolutionFlipsKernelHorizontallyAndVertically()
	{
		minicv::Image image(3, 3);
		SetGrayscalePixels(image, std::array<std::uint8_t, 9>{ 1, 2, 3, 4, 5, 6, 7, 8, 9 });

		const std::vector<double> coefficients{
			1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
		};
		const minicv::ConvolutionKernel kernel(minicv::Size{ 3, 3 }, coefficients);
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 0 };
		const minicv::Image convolvedImage = minicv::CreateConvolvedImage(image, kernel, borderParameters);

		AssertGrayscalePixelsEqual(convolvedImage, std::array<std::uint8_t, 9>{ 5, 6, 0, 8, 9, 0, 0, 0, 0 });
	}

	void TestConstantAndReplicateBordersProduceExpectedValues()
	{
		minicv::Image image(2, 1);
		SetGrayscalePixels(image, std::array<std::uint8_t, 2>{ 30, 60 });

		const minicv::ConvolutionKernel kernel(minicv::Size{ 3, 1 }, std::vector<double>{ 1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0 });
		const minicv::ImageBorderParameters constantBorderParameters{ minicv::EBorderType::CONSTANT, 90 };
		const minicv::ImageBorderParameters replicateBorderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::Image constantBorderImage = minicv::CreateConvolvedImage(image, kernel, constantBorderParameters);
		const minicv::Image replicateBorderImage = minicv::CreateConvolvedImage(image, kernel, replicateBorderParameters);

		AssertGrayscalePixelsEqual(constantBorderImage, std::array<std::uint8_t, 2>{ 60, 60 });
		AssertGrayscalePixelsEqual(replicateBorderImage, std::array<std::uint8_t, 2>{ 40, 50 });
	}

	void TestConvolutionClampsAndRoundsPixelValues()
	{
		minicv::Image image(3, 1);
		SetGrayscalePixels(image, std::array<std::uint8_t, 3>{ 1, 100, 200 });

		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 0 };
		const minicv::ConvolutionKernel positiveKernel(minicv::Size{ 1, 1 }, std::vector<double>{ 1.5 });
		const minicv::ConvolutionKernel negativeKernel(minicv::Size{ 1, 1 }, std::vector<double>{ -1.0 });
		const minicv::Image positiveImage = minicv::CreateConvolvedImage(image, positiveKernel, borderParameters);
		const minicv::Image negativeImage = minicv::CreateConvolvedImage(image, negativeKernel, borderParameters);

		AssertGrayscalePixelsEqual(positiveImage, std::array<std::uint8_t, 3>{ 2, 150, 255 });
		AssertGrayscalePixelsEqual(negativeImage, std::array<std::uint8_t, 3>{ 0, 0, 0 });
	}

	void TestConvolutionProcessesRgbChannelsIndependently()
	{
		minicv::Image image(1, 1, minicv::EImageType::UINT8_RGB);
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) = 10;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) = 20;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) = 30;

		const minicv::ConvolutionKernel kernel(minicv::Size{ 1, 1 }, std::vector<double>{ 2.0 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::Image convolvedImage = minicv::CreateConvolvedImage(image, kernel, borderParameters);

		assert(convolvedImage.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(convolvedImage.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) == 20);
		assert(convolvedImage.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) == 40);
		assert(convolvedImage.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) == 60);
	}

	void TestReplicateBorderHandlesTwoDimensionalCorners()
	{
		minicv::Image image(2, 2);
		SetGrayscalePixels(image, std::array<std::uint8_t, 4>{ 1, 2, 3, 4 });

		const minicv::ConvolutionKernel kernel(minicv::Size{ 3, 3 }, std::vector<double>(9, 1.0));
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::Image convolvedImage = minicv::CreateConvolvedImage(image, kernel, borderParameters);

		AssertGrayscalePixelsEqual(convolvedImage, std::array<std::uint8_t, 4>{ 18, 21, 24, 27 });
	}

	void TestConstantBorderProcessesRgbChannelsIndependently()
	{
		minicv::Image image(1, 1, minicv::EImageType::UINT8_RGB);
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) = 10;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) = 20;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) = 30;

		const minicv::ConvolutionKernel kernel(minicv::Size{ 3, 3 }, std::vector<double>(9, 1.0));
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 5 };
		const minicv::Image convolvedImage = minicv::CreateConvolvedImage(image, kernel, borderParameters);

		assert(convolvedImage.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) == 50);
		assert(convolvedImage.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) == 60);
		assert(convolvedImage.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) == 70);
	}

	void TestConvolutionPreservesEmptyImageShape()
	{
		const minicv::Image image(0, 3, minicv::EImageType::UINT8_RGB);
		const minicv::ConvolutionKernel kernel(minicv::Size{ 3, 3 }, std::vector<double>(9, 1.0));
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::Image convolvedImage = minicv::CreateConvolvedImage(image, kernel, borderParameters);

		assert(convolvedImage.IsEmpty());
		assert(convolvedImage.GetWidth() == 0);
		assert(convolvedImage.GetHeight() == 3);
		assert(convolvedImage.GetImageType() == minicv::EImageType::UINT8_RGB);
	}

	void TestBoxBlurProducesExpectedAverages()
	{
		minicv::Image image(3, 3);
		SetGrayscalePixels(image, std::array<std::uint8_t, 9>{ 0, 10, 20, 30, 40, 50, 60, 70, 80 });

		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 0 };
		const minicv::Image blurredImage = minicv::CreateBoxBlurredImage(image, minicv::Size{ 3, 3 }, borderParameters);

		AssertGrayscalePixelsEqual(blurredImage, std::array<std::uint8_t, 9>{ 9, 17, 13, 23, 40, 30, 22, 37, 27 });
	}

	void TestGaussianKernelIsNormalizedAndSymmetric()
	{
		const minicv::ConvolutionKernel kernel = minicv::CreateGaussianKernel(minicv::Size{ 3, 3 }, 1.0);
		double coefficientSum = 0.0;

		for (int y = 0; y < kernel.GetHeight(); ++y)
		{
			for (int x = 0; x < kernel.GetWidth(); ++x)
			{
				coefficientSum += kernel.GetCoefficient(x, y);
			}
		}

		const bool isCoefficientSumNormalized = std::abs(coefficientSum - 1.0) <= COMPARISON_TOLERANCE;
		const bool areCornerCoefficientsEqual = std::abs(kernel.GetCoefficient(0, 0) - kernel.GetCoefficient(2, 2)) <= COMPARISON_TOLERANCE;
		const bool areEdgeCoefficientsEqual = std::abs(kernel.GetCoefficient(1, 0) - kernel.GetCoefficient(1, 2)) <= COMPARISON_TOLERANCE;

		assert(isCoefficientSumNormalized);
		assert(areCornerCoefficientsEqual);
		assert(areEdgeCoefficientsEqual);
		assert(kernel.GetCoefficient(1, 1) > kernel.GetCoefficient(1, 0));
		assert(kernel.GetCoefficient(1, 0) > kernel.GetCoefficient(0, 0));

		(void)isCoefficientSumNormalized;
		(void)areCornerCoefficientsEqual;
		(void)areEdgeCoefficientsEqual;
	}

	void TestGaussianBlurProducesExpectedImpulseResponse()
	{
		minicv::Image image(3, 3);
		image.Fill(0);
		image.GetGrayscalePixel(1, 1) = 255;

		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 0 };
		const minicv::Image blurredImage = minicv::CreateGaussianBlurredImage(image, minicv::Size{ 3, 3 }, 1.0, borderParameters);

		AssertGrayscalePixelsEqual(blurredImage, std::array<std::uint8_t, 9>{ 19, 32, 19, 32, 52, 32, 19, 32, 19 });
	}

	void TestGaussianBlurPreservesSinglePixelRgbImage()
	{
		minicv::Image image(1, 1, minicv::EImageType::UINT8_RGB);
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) = 10;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) = 20;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) = 30;

		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::Image blurredImage = minicv::CreateGaussianBlurredImage(image, minicv::Size{ 3, 3 }, 1.0, borderParameters);

		assert(blurredImage.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(blurredImage.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) == 10);
		assert(blurredImage.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) == 20);
		assert(blurredImage.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) == 30);
	}

	void TestMedianFilterRemovesIsolatedNoise()
	{
		minicv::Image image(3, 3);
		image.Fill(0);
		image.GetGrayscalePixel(1, 1) = 255;

		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::Image filteredImage = minicv::CreateMedianFilteredImage(image, minicv::Size{ 3, 3 }, borderParameters);

		AssertGrayscalePixelsEqual(filteredImage, std::array<std::uint8_t, 9>{ 0, 0, 0, 0, 0, 0, 0, 0, 0 });
		assert(image.GetGrayscalePixel(1, 1) == 255);
	}

	void TestMedianFilterAppliesConstantAndReplicateBorders()
	{
		minicv::Image image(2, 1);
		SetGrayscalePixels(image, std::array<std::uint8_t, 2>{ 10, 200 });

		const minicv::ImageBorderParameters constantBorderParameters{ minicv::EBorderType::CONSTANT, 0 };
		const minicv::ImageBorderParameters replicateBorderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::Image constantBorderImage = minicv::CreateMedianFilteredImage(image, minicv::Size{ 3, 1 }, constantBorderParameters);
		const minicv::Image replicateBorderImage = minicv::CreateMedianFilteredImage(image, minicv::Size{ 3, 1 }, replicateBorderParameters);

		AssertGrayscalePixelsEqual(constantBorderImage, std::array<std::uint8_t, 2>{ 10, 10 });
		AssertGrayscalePixelsEqual(replicateBorderImage, std::array<std::uint8_t, 2>{ 10, 200 });
	}

	void TestMedianFilterProcessesRgbChannelsIndependently()
	{
		minicv::Image image(3, 1, minicv::EImageType::UINT8_RGB);
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) = 0;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) = 255;
		image.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) = 10;
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::RED) = 255;
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::GREEN) = 0;
		image.GetRgbPixel(1, 0, minicv::ERgbChannel::BLUE) = 20;
		image.GetRgbPixel(2, 0, minicv::ERgbChannel::RED) = 0;
		image.GetRgbPixel(2, 0, minicv::ERgbChannel::GREEN) = 255;
		image.GetRgbPixel(2, 0, minicv::ERgbChannel::BLUE) = 30;

		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::Image filteredImage = minicv::CreateMedianFilteredImage(image, minicv::Size{ 3, 1 }, borderParameters);

		assert(filteredImage.GetImageType() == minicv::EImageType::UINT8_RGB);
		assert(filteredImage.GetRgbPixel(0, 0, minicv::ERgbChannel::RED) == 0);
		assert(filteredImage.GetRgbPixel(0, 0, minicv::ERgbChannel::GREEN) == 255);
		assert(filteredImage.GetRgbPixel(0, 0, minicv::ERgbChannel::BLUE) == 10);
		assert(filteredImage.GetRgbPixel(1, 0, minicv::ERgbChannel::RED) == 0);
		assert(filteredImage.GetRgbPixel(1, 0, minicv::ERgbChannel::GREEN) == 255);
		assert(filteredImage.GetRgbPixel(1, 0, minicv::ERgbChannel::BLUE) == 20);
		assert(filteredImage.GetRgbPixel(2, 0, minicv::ERgbChannel::RED) == 0);
		assert(filteredImage.GetRgbPixel(2, 0, minicv::ERgbChannel::GREEN) == 255);
		assert(filteredImage.GetRgbPixel(2, 0, minicv::ERgbChannel::BLUE) == 30);
	}

	void TestMedianFilterPreservesEmptyImageShape()
	{
		const minicv::Image image(0, 3, minicv::EImageType::UINT8_RGB);
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::Image filteredImage = minicv::CreateMedianFilteredImage(image, minicv::Size{ 3, 3 }, borderParameters);

		assert(filteredImage.IsEmpty());
		assert(filteredImage.GetWidth() == 0);
		assert(filteredImage.GetHeight() == 3);
		assert(filteredImage.GetImageType() == minicv::EImageType::UINT8_RGB);
	}

	void TestSharpeningEnhancesCenterPixel()
	{
		minicv::Image image(3, 3);
		image.Fill(0);
		image.GetGrayscalePixel(1, 1) = 50;

		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 0 };
		const minicv::Image sharpenedImage = minicv::CreateSharpenedImage(image, borderParameters);

		AssertGrayscalePixelsEqual(sharpenedImage, std::array<std::uint8_t, 9>{ 0, 0, 0, 0, 250, 0, 0, 0, 0 });
	}

	void TestLaplacianResponsePreservesSignedValues()
	{
		minicv::Image image(3, 3);
		image.Fill(0);
		image.GetGrayscalePixel(1, 1) = 100;

		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 0 };
		const minicv::GrayscaleFilterResponse response = minicv::CreateLaplacianResponse(image, borderParameters);

		assert(response.GetSize().Width == 3);
		assert(response.GetSize().Height == 3);
		assert(response.GetResponseValue(0, 0) == 0.0);
		assert(response.GetResponseValue(1, 0) == 100.0);
		assert(response.GetResponseValue(2, 0) == 0.0);
		assert(response.GetResponseValue(0, 1) == 100.0);
		assert(response.GetResponseValue(1, 1) == -400.0);
		assert(response.GetResponseValue(2, 1) == 100.0);
		assert(response.GetResponseValue(0, 2) == 0.0);
		assert(response.GetResponseValue(1, 2) == 100.0);
		assert(response.GetResponseValue(2, 2) == 0.0);
		assert(image.GetGrayscalePixel(1, 1) == 100);
	}

	void TestLaplacianResponsePreservesEmptyImageSize()
	{
		const minicv::Image image(0, 4);
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::GrayscaleFilterResponse response = minicv::CreateLaplacianResponse(image, borderParameters);

		assert(response.IsEmpty());
		assert(response.GetWidth() == 0);
		assert(response.GetHeight() == 4);
	}

	void TestSignedResponseImageMapsNegativeZeroAndPositiveValues()
	{
		minicv::GrayscaleFilterResponse response(minicv::Size{ 5, 1 });
		response.GetResponseValue(0, 0) = -20.0;
		response.GetResponseValue(1, 0) = -10.0;
		response.GetResponseValue(2, 0) = 0.0;
		response.GetResponseValue(3, 0) = 10.0;
		response.GetResponseValue(4, 0) = 20.0;

		const minicv::Image signedResponseImage = minicv::CreateSignedResponseImage(response);

		AssertGrayscalePixelsEqual(signedResponseImage, std::array<std::uint8_t, 5>{ 0, 64, 128, 191, 255 });
	}

	void TestSignedResponseImageCentersZeroResponse()
	{
		const minicv::GrayscaleFilterResponse response(minicv::Size{ 3, 2 });
		const minicv::Image signedResponseImage = minicv::CreateSignedResponseImage(response);

		AssertGrayscalePixelsEqual(signedResponseImage, std::array<std::uint8_t, 6>{ 128, 128, 128, 128, 128, 128 });
	}

	void TestSignedResponseImagePreservesEmptyResponseSize()
	{
		const minicv::GrayscaleFilterResponse response(minicv::Size{ 0, 4 });
		const minicv::Image signedResponseImage = minicv::CreateSignedResponseImage(response);

		assert(signedResponseImage.IsEmpty());
		assert(signedResponseImage.GetWidth() == 0);
		assert(signedResponseImage.GetHeight() == 4);
		assert(signedResponseImage.GetImageType() == minicv::EImageType::UINT8_GRAYSCALE);
	}
}

void RunImageFilteringTests()
{
	TestIdentityConvolutionPreservesImage();
	TestConvolutionFlipsKernelHorizontallyAndVertically();
	TestConstantAndReplicateBordersProduceExpectedValues();
	TestConvolutionClampsAndRoundsPixelValues();
	TestConvolutionProcessesRgbChannelsIndependently();
	TestReplicateBorderHandlesTwoDimensionalCorners();
	TestConstantBorderProcessesRgbChannelsIndependently();
	TestConvolutionPreservesEmptyImageShape();
	TestBoxBlurProducesExpectedAverages();
	TestGaussianKernelIsNormalizedAndSymmetric();
	TestGaussianBlurProducesExpectedImpulseResponse();
	TestGaussianBlurPreservesSinglePixelRgbImage();
	TestMedianFilterRemovesIsolatedNoise();
	TestMedianFilterAppliesConstantAndReplicateBorders();
	TestMedianFilterProcessesRgbChannelsIndependently();
	TestMedianFilterPreservesEmptyImageShape();
	TestSharpeningEnhancesCenterPixel();
	TestLaplacianResponsePreservesSignedValues();
	TestLaplacianResponsePreservesEmptyImageSize();
	TestSignedResponseImageMapsNegativeZeroAndPositiveValues();
	TestSignedResponseImageCentersZeroResponse();
	TestSignedResponseImagePreservesEmptyResponseSize();
}
