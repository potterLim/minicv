#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

#include "minicv/ConvolutionKernel.h"
#include "minicv/ImageFiltering.h"

namespace minicv
{
	namespace
	{
		constexpr std::size_t GRAYSCALE_CHANNEL_INDEX = 0;

		bool IsSupportedBorderType(const EBorderType borderType)
		{
			return borderType == EBorderType::CONSTANT || borderType == EBorderType::REPLICATE;
		}

		std::size_t CalculateKernelCoefficientCount(const Size kernelSize)
		{
			assert(kernelSize.Width > 0 && "kernel width must be positive.");
			assert(kernelSize.Height > 0 && "kernel height must be positive.");
			assert(kernelSize.Width % 2 == 1 && "kernel width must be odd.");
			assert(kernelSize.Height % 2 == 1 && "kernel height must be odd.");

			const std::size_t kernelWidth = static_cast<std::size_t>(kernelSize.Width);
			const std::size_t kernelHeight = static_cast<std::size_t>(kernelSize.Height);

			assert(kernelWidth <= std::numeric_limits<std::size_t>::max() / kernelHeight && "kernel coefficient count overflow.");

			return kernelWidth * kernelHeight;
		}

		std::vector<double> CreateKernelCoefficientBuffer(const std::size_t coefficientCount)
		{
			std::vector<double> coefficients;
			assert(coefficientCount <= coefficients.max_size() && "kernel coefficient count exceeds maximum vector size.");

			coefficients.resize(coefficientCount);
			return coefficients;
		}

		ConvolutionKernel CreateSharpeningKernel()
		{
			return ConvolutionKernel(
				Size{ 3, 3 },
				std::vector<double>{ 0.0, -1.0, 0.0, -1.0, 5.0, -1.0, 0.0, -1.0, 0.0 });
		}

		ConvolutionKernel CreateLaplacianKernel()
		{
			return ConvolutionKernel(
				Size{ 3, 3 },
				std::vector<double>{ 0.0, 1.0, 0.0, 1.0, -4.0, 1.0, 0.0, 1.0, 0.0 });
		}

		bool IsInsideImage(const Image& image, const std::int64_t x, const std::int64_t y)
		{
			return x >= 0 && x < image.GetWidth() && y >= 0 && y < image.GetHeight();
		}

		int ClampImageCoordinate(const std::int64_t coordinate, const int imageLength)
		{
			assert(imageLength > 0 && "image length must be positive.");

			if (coordinate < 0)
			{
				return 0;
			}

			const int maximumCoordinate = imageLength - 1;
			if (coordinate > maximumCoordinate)
			{
				return maximumCoordinate;
			}

			return static_cast<int>(coordinate);
		}

		std::size_t CalculatePixelByteIndex(const Image& image, const int x, const int y, const std::size_t channelIndex)
		{
			const std::size_t rowOffset = static_cast<std::size_t>(y) * static_cast<std::size_t>(image.GetBytesPerRow());
			const std::size_t columnOffset = static_cast<std::size_t>(x) * static_cast<std::size_t>(image.GetChannelCount());

			return rowOffset + columnOffset + channelIndex;
		}

		std::uint8_t GetPixelValue(const Image& image, const int x, const int y, const std::size_t channelIndex)
		{
			const std::size_t pixelByteIndex = CalculatePixelByteIndex(image, x, y, channelIndex);
			return image.GetPixelData()[pixelByteIndex];
		}

		std::uint8_t GetBorderedPixelValue(
			const Image& image,
			const std::int64_t x,
			const std::int64_t y,
			const std::size_t channelIndex,
			const ImageBorderParameters borderParameters)
		{
			if (IsInsideImage(image, x, y))
			{
				return GetPixelValue(image, static_cast<int>(x), static_cast<int>(y), channelIndex);
			}

			switch (borderParameters.BorderType)
			{
			case EBorderType::CONSTANT:
				return borderParameters.ConstantBorderValue;

			case EBorderType::REPLICATE:
			{
				const int clampedX = ClampImageCoordinate(x, image.GetWidth());
				const int clampedY = ClampImageCoordinate(y, image.GetHeight());
				return GetPixelValue(image, clampedX, clampedY, channelIndex);
			}

			default:
				assert(false && "unsupported border type.");
				return 0;
			}
		}

		std::uint8_t ClampAndRoundToByte(const double pixelValue)
		{
			const std::uint8_t minimumByteValue = std::numeric_limits<std::uint8_t>::min();
			const std::uint8_t maximumByteValue = std::numeric_limits<std::uint8_t>::max();

			if (pixelValue <= static_cast<double>(minimumByteValue))
			{
				return minimumByteValue;
			}

			if (pixelValue >= static_cast<double>(maximumByteValue))
			{
				return maximumByteValue;
			}

			const bool isPixelValueFinite = std::isfinite(pixelValue);
			assert(isPixelValueFinite && "convolved pixel value must be finite.");

			if (!isPixelValueFinite)
			{
				return minimumByteValue;
			}

			return static_cast<std::uint8_t>(std::lround(pixelValue));
		}

		double ConvolvePixelChannel(
			const Image& image,
			const ConvolutionKernel& kernel,
			const int x,
			const int y,
			const std::size_t channelIndex,
			const ImageBorderParameters borderParameters)
		{
			const int kernelCenterX = kernel.GetWidth() / 2;
			const int kernelCenterY = kernel.GetHeight() / 2;
			double convolvedPixelValue = 0.0;

			for (int kernelY = 0; kernelY < kernel.GetHeight(); ++kernelY)
			{
				for (int kernelX = 0; kernelX < kernel.GetWidth(); ++kernelX)
				{
					const std::int64_t sourceX = static_cast<std::int64_t>(x) + kernelCenterX - kernelX;
					const std::int64_t sourceY = static_cast<std::int64_t>(y) + kernelCenterY - kernelY;
					const std::uint8_t sourcePixelValue = GetBorderedPixelValue(image, sourceX, sourceY, channelIndex, borderParameters);
					const double coefficient = kernel.GetCoefficient(kernelX, kernelY);

					convolvedPixelValue += static_cast<double>(sourcePixelValue) * coefficient;
				}
			}

			return convolvedPixelValue;
		}
	}

	Image CreateConvolvedImage(const Image& image, const ConvolutionKernel& kernel, const ImageBorderParameters borderParameters)
	{
		const bool isBorderTypeSupported = IsSupportedBorderType(borderParameters.BorderType);
		assert(isBorderTypeSupported && "border type must be CONSTANT or REPLICATE.");

		(void)isBorderTypeSupported;

		Image convolvedImage(image.GetSize(), image.GetImageType());
		if (image.IsEmpty())
		{
			return convolvedImage;
		}

		std::uint8_t* const convolvedPixelData = convolvedImage.GetPixelData();
		const std::size_t channelCount = static_cast<std::size_t>(image.GetChannelCount());

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				for (std::size_t channelIndex = 0; channelIndex < channelCount; ++channelIndex)
				{
					const double convolvedPixelValue = ConvolvePixelChannel(image, kernel, x, y, channelIndex, borderParameters);
					const std::size_t pixelByteIndex = CalculatePixelByteIndex(convolvedImage, x, y, channelIndex);

					convolvedPixelData[pixelByteIndex] = ClampAndRoundToByte(convolvedPixelValue);
				}
			}
		}

		return convolvedImage;
	}

	Image CreateBoxBlurredImage(const Image& image, const Size kernelSize, const ImageBorderParameters borderParameters)
	{
		const std::size_t coefficientCount = CalculateKernelCoefficientCount(kernelSize);
		const double coefficient = 1.0 / static_cast<double>(coefficientCount);
		std::vector<double> coefficients = CreateKernelCoefficientBuffer(coefficientCount);

		for (double& currentCoefficient : coefficients)
		{
			currentCoefficient = coefficient;
		}

		const ConvolutionKernel boxKernel(kernelSize, std::move(coefficients));
		return CreateConvolvedImage(image, boxKernel, borderParameters);
	}

	ConvolutionKernel CreateGaussianKernel(const Size kernelSize, const double standardDeviation)
	{
		const bool isStandardDeviationFinite = std::isfinite(standardDeviation);
		assert(isStandardDeviationFinite && "standard deviation must be finite.");
		assert(standardDeviation > 0.0 && "standard deviation must be positive.");

		(void)isStandardDeviationFinite;

		const std::size_t coefficientCount = CalculateKernelCoefficientCount(kernelSize);
		std::vector<double> coefficients = CreateKernelCoefficientBuffer(coefficientCount);
		const int kernelCenterX = kernelSize.Width / 2;
		const int kernelCenterY = kernelSize.Height / 2;
		double coefficientSum = 0.0;

		for (int kernelY = 0; kernelY < kernelSize.Height; ++kernelY)
		{
			for (int kernelX = 0; kernelX < kernelSize.Width; ++kernelX)
			{
				const double normalizedX = static_cast<double>(kernelX - kernelCenterX) / standardDeviation;
				const double normalizedY = static_cast<double>(kernelY - kernelCenterY) / standardDeviation;
				const double exponent = -0.5 * (normalizedX * normalizedX + normalizedY * normalizedY);
				const double coefficient = std::exp(exponent);
				const std::size_t rowOffset = static_cast<std::size_t>(kernelY) * static_cast<std::size_t>(kernelSize.Width);
				const std::size_t coefficientIndex = rowOffset + static_cast<std::size_t>(kernelX);

				coefficients[coefficientIndex] = coefficient;
				coefficientSum += coefficient;
			}
		}

		const bool isCoefficientSumFinite = std::isfinite(coefficientSum);
		assert(isCoefficientSumFinite && coefficientSum > 0.0 && "gaussian coefficient sum must be positive and finite.");

		(void)isCoefficientSumFinite;

		for (double& coefficient : coefficients)
		{
			coefficient /= coefficientSum;
		}

		return ConvolutionKernel(kernelSize, std::move(coefficients));
	}

	Image CreateGaussianBlurredImage(
		const Image& image,
		const Size kernelSize,
		const double standardDeviation,
		const ImageBorderParameters borderParameters)
	{
		const ConvolutionKernel gaussianKernel = CreateGaussianKernel(kernelSize, standardDeviation);
		return CreateConvolvedImage(image, gaussianKernel, borderParameters);
	}

	Image CreateSharpenedImage(const Image& image, const ImageBorderParameters borderParameters)
	{
		const ConvolutionKernel sharpeningKernel = CreateSharpeningKernel();
		return CreateConvolvedImage(image, sharpeningKernel, borderParameters);
	}

	GrayscaleFilterResponse CreateLaplacianResponse(const Image& grayscaleImage, const ImageBorderParameters borderParameters)
	{
		const bool isImageGrayscale = grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE;
		assert(isImageGrayscale && "image type must be UINT8_GRAYSCALE.");

		const bool isBorderTypeSupported = IsSupportedBorderType(borderParameters.BorderType);
		assert(isBorderTypeSupported && "border type must be CONSTANT or REPLICATE.");

		(void)isImageGrayscale;
		(void)isBorderTypeSupported;

		GrayscaleFilterResponse response(grayscaleImage.GetSize());
		if (grayscaleImage.IsEmpty())
		{
			return response;
		}

		const ConvolutionKernel laplacianKernel = CreateLaplacianKernel();

		for (int y = 0; y < grayscaleImage.GetHeight(); ++y)
		{
			for (int x = 0; x < grayscaleImage.GetWidth(); ++x)
			{
				response.GetResponseValue(x, y) = ConvolvePixelChannel(
					grayscaleImage,
					laplacianKernel,
					x,
					y,
					GRAYSCALE_CHANNEL_INDEX,
					borderParameters);
			}
		}

		return response;
	}
}
