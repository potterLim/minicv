#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <vector>

#include "ImageFilteringTest.h"
#include "minicv/ConvolutionKernel.h"
#include "minicv/GrayscaleFilterResponse.h"
#include "minicv/Image.h"
#include "minicv/ImageFiltering.h"

namespace
{
	constexpr double COMPARISON_TOLERANCE = 1e-12;
	constexpr double FIRST_QUADRANT_DIRECTION_RADIANS = 0.9272952180016122;
	constexpr double HALF_PI_RADIANS = std::numbers::pi_v<double> / 2.0;
	constexpr double SECOND_QUADRANT_DIRECTION_RADIANS = std::numbers::pi_v<double> - FIRST_QUADRANT_DIRECTION_RADIANS;

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

	template <std::size_t RESPONSE_VALUE_COUNT>
	void SetFilterResponseValues(minicv::GrayscaleFilterResponse& response, const std::array<double, RESPONSE_VALUE_COUNT>& responseValues)
	{
		const std::size_t responseValueCount = static_cast<std::size_t>(response.GetWidth()) * static_cast<std::size_t>(response.GetHeight());
		const bool doesResponseValueCountMatch = responseValueCount == RESPONSE_VALUE_COUNT;
		assert(doesResponseValueCountMatch);

		(void)doesResponseValueCountMatch;

		const std::size_t responseWidth = static_cast<std::size_t>(response.GetWidth());

		for (std::size_t responseValueIndex = 0; responseValueIndex < RESPONSE_VALUE_COUNT; ++responseValueIndex)
		{
			const int x = static_cast<int>(responseValueIndex % responseWidth);
			const int y = static_cast<int>(responseValueIndex / responseWidth);

			response.GetResponseValue(x, y) = responseValues[responseValueIndex];
		}
	}

	template <std::size_t RESPONSE_VALUE_COUNT>
	void AssertFilterResponseValuesEqual(
		const minicv::GrayscaleFilterResponse& response,
		const std::array<double, RESPONSE_VALUE_COUNT>& expectedResponseValues)
	{
		const std::size_t responseValueCount = static_cast<std::size_t>(response.GetWidth()) * static_cast<std::size_t>(response.GetHeight());
		const bool doesResponseValueCountMatch = responseValueCount == RESPONSE_VALUE_COUNT;
		assert(doesResponseValueCountMatch);

		(void)doesResponseValueCountMatch;

		const std::size_t responseWidth = static_cast<std::size_t>(response.GetWidth());

		for (std::size_t responseValueIndex = 0; responseValueIndex < RESPONSE_VALUE_COUNT; ++responseValueIndex)
		{
			const int x = static_cast<int>(responseValueIndex % responseWidth);
			const int y = static_cast<int>(responseValueIndex / responseWidth);
			const double responseValue = response.GetResponseValue(x, y);
			const bool isResponseValueEqual = std::abs(responseValue - expectedResponseValues[responseValueIndex]) <= COMPARISON_TOLERANCE;
			assert(isResponseValueEqual);

			(void)isResponseValueEqual;
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

	void TestSobelResponsesDetectHorizontalAndVerticalChanges()
	{
		minicv::Image image(3, 3);
		SetGrayscalePixels(image, std::array<std::uint8_t, 9>{ 0, 10, 20, 20, 30, 40, 40, 50, 60 });

		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::GrayscaleFilterResponse sobelXResponse = minicv::CreateSobelXResponse(image, borderParameters);
		const minicv::GrayscaleFilterResponse sobelYResponse = minicv::CreateSobelYResponse(image, borderParameters);

		AssertFilterResponseValuesEqual(sobelXResponse, std::array<double, 9>{ 40.0, 80.0, 40.0, 40.0, 80.0, 40.0, 40.0, 80.0, 40.0 });
		AssertFilterResponseValuesEqual(sobelYResponse, std::array<double, 9>{ 80.0, 80.0, 80.0, 160.0, 160.0, 160.0, 80.0, 80.0, 80.0 });
	}

	void TestGradientMagnitudeAndDirectionResponses()
	{
		minicv::GrayscaleFilterResponse sobelXResponse(minicv::Size{ 5, 1 });
		minicv::GrayscaleFilterResponse sobelYResponse(minicv::Size{ 5, 1 });
		SetFilterResponseValues(sobelXResponse, std::array<double, 5>{ 3.0, 0.0, -3.0, 0.0, 0.0 });
		SetFilterResponseValues(sobelYResponse, std::array<double, 5>{ 4.0, 5.0, 4.0, -5.0, 0.0 });

		const minicv::GrayscaleFilterResponse gradientMagnitudeResponse = minicv::CreateGradientMagnitudeResponse(sobelXResponse, sobelYResponse);
		const minicv::GrayscaleFilterResponse gradientDirectionResponse = minicv::CreateGradientDirectionResponse(sobelXResponse, sobelYResponse);

		AssertFilterResponseValuesEqual(gradientMagnitudeResponse, std::array<double, 5>{ 5.0, 5.0, 5.0, 5.0, 0.0 });
		AssertFilterResponseValuesEqual(
			gradientDirectionResponse,
			std::array<double, 5>{ FIRST_QUADRANT_DIRECTION_RADIANS, HALF_PI_RADIANS, SECOND_QUADRANT_DIRECTION_RADIANS, -HALF_PI_RADIANS, 0.0 });
	}

	void TestGradientMagnitudeNormalizationMapsFullByteRange()
	{
		minicv::GrayscaleFilterResponse gradientMagnitudeResponse(minicv::Size{ 5, 1 });
		SetFilterResponseValues(gradientMagnitudeResponse, std::array<double, 5>{ 0.0, 5.0, 10.0, 15.0, 20.0 });

		const minicv::Image normalizedImage = minicv::CreateNormalizedGradientMagnitudeImage(gradientMagnitudeResponse);

		AssertGrayscalePixelsEqual(normalizedImage, std::array<std::uint8_t, 5>{ 0, 64, 128, 191, 255 });
	}

	void TestZeroGradientMagnitudeNormalizesToBlack()
	{
		const minicv::GrayscaleFilterResponse gradientMagnitudeResponse(minicv::Size{ 3, 2 });
		const minicv::Image normalizedImage = minicv::CreateNormalizedGradientMagnitudeImage(gradientMagnitudeResponse);

		AssertGrayscalePixelsEqual(normalizedImage, std::array<std::uint8_t, 6>{ 0, 0, 0, 0, 0, 0 });
	}

	void TestSobelConstantBorderPreservesSignedCornerResponses()
	{
		minicv::Image image(2, 2);
		SetGrayscalePixels(image, std::array<std::uint8_t, 4>{ 10, 20, 30, 40 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 5 };
		const minicv::GrayscaleFilterResponse sobelXResponse = minicv::CreateSobelXResponse(image, borderParameters);
		const minicv::GrayscaleFilterResponse sobelYResponse = minicv::CreateSobelYResponse(image, borderParameters);

		AssertFilterResponseValuesEqual(sobelXResponse, std::array<double, 4>{ 65.0, -35.0, 85.0, -55.0 });
		AssertFilterResponseValuesEqual(sobelYResponse, std::array<double, 4>{ 85.0, 95.0, -25.0, -35.0 });
		AssertGrayscalePixelsEqual(image, std::array<std::uint8_t, 4>{ 10, 20, 30, 40 });
	}

	void TestSobelReplicateBorderHandlesSingleRowAndColumn()
	{
		minicv::Image rowImage(3, 1);
		minicv::Image columnImage(1, 3);
		SetGrayscalePixels(rowImage, std::array<std::uint8_t, 3>{ 255, 128, 0 });
		SetGrayscalePixels(columnImage, std::array<std::uint8_t, 3>{ 255, 128, 0 });
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 99 };

		AssertFilterResponseValuesEqual(minicv::CreateSobelXResponse(rowImage, borderParameters),
			std::array<double, 3>{ -508.0, -1020.0, -512.0 });
		AssertFilterResponseValuesEqual(minicv::CreateSobelYResponse(rowImage, borderParameters),
			std::array<double, 3>{ 0.0, 0.0, 0.0 });
		AssertFilterResponseValuesEqual(minicv::CreateSobelXResponse(columnImage, borderParameters),
			std::array<double, 3>{ 0.0, 0.0, 0.0 });
		AssertFilterResponseValuesEqual(minicv::CreateSobelYResponse(columnImage, borderParameters),
			std::array<double, 3>{ -508.0, -1020.0, -512.0 });
	}

	void TestLaplacianBordersProduceExpectedCornerResponses()
	{
		minicv::Image image(2, 2);
		SetGrayscalePixels(image, std::array<std::uint8_t, 4>{ 10, 20, 30, 40 });
		const minicv::ImageBorderParameters constantBorderParameters{ minicv::EBorderType::CONSTANT, 5 };
		const minicv::ImageBorderParameters replicateBorderParameters{ minicv::EBorderType::REPLICATE, 99 };

		AssertFilterResponseValuesEqual(minicv::CreateLaplacianResponse(image, constantBorderParameters),
			std::array<double, 4>{ 20.0, -20.0, -60.0, -100.0 });
		AssertFilterResponseValuesEqual(minicv::CreateLaplacianResponse(image, replicateBorderParameters),
			std::array<double, 4>{ 30.0, 10.0, -10.0, -30.0 });
	}

	void TestFiltersHandleKernelsLargerThanSinglePixelImage()
	{
		minicv::Image image(1, 1);
		image.Fill(90);
		const minicv::Size kernelSize{ 5, 5 };
		const minicv::ImageBorderParameters constantBorderParameters{ minicv::EBorderType::CONSTANT, 15 };
		const minicv::ImageBorderParameters replicateBorderParameters{ minicv::EBorderType::REPLICATE, 15 };

		AssertGrayscalePixelsEqual(minicv::CreateBoxBlurredImage(image, kernelSize, constantBorderParameters),
			std::array<std::uint8_t, 1>{ 18 });
		AssertGrayscalePixelsEqual(minicv::CreateMedianFilteredImage(image, kernelSize, constantBorderParameters),
			std::array<std::uint8_t, 1>{ 15 });
		AssertGrayscalePixelsEqual(minicv::CreateBoxBlurredImage(image, kernelSize, replicateBorderParameters),
			std::array<std::uint8_t, 1>{ 90 });
		AssertGrayscalePixelsEqual(minicv::CreateGaussianBlurredImage(image, kernelSize, 1.0, replicateBorderParameters),
			std::array<std::uint8_t, 1>{ 90 });
		AssertGrayscalePixelsEqual(minicv::CreateMedianFilteredImage(image, kernelSize, replicateBorderParameters),
			std::array<std::uint8_t, 1>{ 90 });
		AssertGrayscalePixelsEqual(minicv::CreateSharpenedImage(image, replicateBorderParameters),
			std::array<std::uint8_t, 1>{ 90 });
		AssertFilterResponseValuesEqual(minicv::CreateSobelXResponse(image, replicateBorderParameters),
			std::array<double, 1>{ 0.0 });
		AssertFilterResponseValuesEqual(minicv::CreateSobelYResponse(image, replicateBorderParameters),
			std::array<double, 1>{ 0.0 });
	}

	void TestGaussianAndSharpeningConstantBordersProduceExpectedPixels()
	{
		minicv::Image image(1, 1);
		image.Fill(90);
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::CONSTANT, 15 };

		// The center weight of the normalized 3x3 Gaussian at sigma=1 is approximately 0.20418.
		AssertGrayscalePixelsEqual(minicv::CreateGaussianBlurredImage(image, minicv::Size{ 3, 3 }, 1.0, borderParameters),
			std::array<std::uint8_t, 1>{ 30 });
		AssertGrayscalePixelsEqual(minicv::CreateSharpenedImage(image, borderParameters),
			std::array<std::uint8_t, 1>{ 255 });
	}

	void TestSignedResponseImageUsesMaximumAbsoluteValue()
	{
		minicv::GrayscaleFilterResponse response(minicv::Size{ 4, 1 });
		SetFilterResponseValues(response, std::array<double, 4>{ -40.0, -20.0, 0.0, 10.0 });
		const minicv::Image signedResponseImage = minicv::CreateSignedResponseImage(response);

		AssertGrayscalePixelsEqual(signedResponseImage, std::array<std::uint8_t, 4>{ 0, 64, 128, 159 });
		AssertFilterResponseValuesEqual(response, std::array<double, 4>{ -40.0, -20.0, 0.0, 10.0 });
	}

	void TestGradientDirectionCoversNegativeQuadrantsAndAxes()
	{
		minicv::GrayscaleFilterResponse sobelXResponse(minicv::Size{ 4, 1 });
		minicv::GrayscaleFilterResponse sobelYResponse(minicv::Size{ 4, 1 });
		SetFilterResponseValues(sobelXResponse, std::array<double, 4>{ -3.0, 3.0, -5.0, 5.0 });
		SetFilterResponseValues(sobelYResponse, std::array<double, 4>{ -4.0, -4.0, 0.0, 0.0 });

		AssertFilterResponseValuesEqual(minicv::CreateGradientDirectionResponse(sobelXResponse, sobelYResponse),
			std::array<double, 4>{ -SECOND_QUADRANT_DIRECTION_RADIANS, -FIRST_QUADRANT_DIRECTION_RADIANS, std::numbers::pi_v<double>, 0.0 });
	}

	void TestGradientNormalizationPreservesZeroOrigin()
	{
		minicv::GrayscaleFilterResponse response(minicv::Size{ 3, 1 });
		SetFilterResponseValues(response, std::array<double, 3>{ 5.0, 10.0, 20.0 });
		AssertGrayscalePixelsEqual(minicv::CreateNormalizedGradientMagnitudeImage(response),
			std::array<std::uint8_t, 3>{ 64, 128, 255 });
		AssertFilterResponseValuesEqual(response, std::array<double, 3>{ 5.0, 10.0, 20.0 });
	}

	void TestSobelPipelinePreservesEmptySize()
	{
		const minicv::Image image(0, 4);
		const minicv::ImageBorderParameters borderParameters{ minicv::EBorderType::REPLICATE, 0 };
		const minicv::GrayscaleFilterResponse sobelXResponse = minicv::CreateSobelXResponse(image, borderParameters);
		const minicv::GrayscaleFilterResponse sobelYResponse = minicv::CreateSobelYResponse(image, borderParameters);
		const minicv::GrayscaleFilterResponse gradientMagnitudeResponse = minicv::CreateGradientMagnitudeResponse(sobelXResponse, sobelYResponse);
		const minicv::GrayscaleFilterResponse gradientDirectionResponse = minicv::CreateGradientDirectionResponse(sobelXResponse, sobelYResponse);
		const minicv::Image normalizedImage = minicv::CreateNormalizedGradientMagnitudeImage(gradientMagnitudeResponse);

		assert(sobelXResponse.IsEmpty());
		assert(sobelYResponse.IsEmpty());
		assert(gradientMagnitudeResponse.IsEmpty());
		assert(gradientDirectionResponse.IsEmpty());
		assert(normalizedImage.IsEmpty());
		assert(normalizedImage.GetWidth() == 0);
		assert(normalizedImage.GetHeight() == 4);
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
	TestSobelResponsesDetectHorizontalAndVerticalChanges();
	TestGradientMagnitudeAndDirectionResponses();
	TestGradientMagnitudeNormalizationMapsFullByteRange();
	TestZeroGradientMagnitudeNormalizesToBlack();
	TestSobelPipelinePreservesEmptySize();
	TestSobelConstantBorderPreservesSignedCornerResponses();
	TestSobelReplicateBorderHandlesSingleRowAndColumn();
	TestLaplacianBordersProduceExpectedCornerResponses();
	TestFiltersHandleKernelsLargerThanSinglePixelImage();
	TestGaussianAndSharpeningConstantBordersProduceExpectedPixels();
	TestSignedResponseImageUsesMaximumAbsoluteValue();
	TestGradientDirectionCoversNegativeQuadrantsAndAxes();
	TestGradientNormalizationPreservesZeroOrigin();
}
