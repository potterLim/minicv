#pragma once

#include "minicv/ConvolutionKernel.h"
#include "minicv/GrayscaleFilterResponse.h"
#include "minicv/Image.h"
#include "minicv/ImageBorderParameters.h"
#include "minicv/Size.h"

namespace minicv
{
	[[nodiscard]] Image CreateConvolvedImage(const Image& image, const ConvolutionKernel& kernel, const ImageBorderParameters borderParameters);

	[[nodiscard]] Image CreateBoxBlurredImage(const Image& image, const Size kernelSize, const ImageBorderParameters borderParameters);
	[[nodiscard]] ConvolutionKernel CreateGaussianKernel(const Size kernelSize, const double standardDeviation);
	[[nodiscard]] Image CreateGaussianBlurredImage(
		const Image& image,
		const Size kernelSize,
		const double standardDeviation,
		const ImageBorderParameters borderParameters);
	[[nodiscard]] Image CreateMedianFilteredImage(const Image& image, const Size kernelSize, const ImageBorderParameters borderParameters);
	[[nodiscard]] Image CreateSharpenedImage(const Image& image, const ImageBorderParameters borderParameters);
	[[nodiscard]] GrayscaleFilterResponse CreateLaplacianResponse(const Image& grayscaleImage, const ImageBorderParameters borderParameters);
	[[nodiscard]] Image CreateSignedResponseImage(const GrayscaleFilterResponse& response);

	[[nodiscard]] GrayscaleFilterResponse CreateSobelXResponse(const Image& grayscaleImage, const ImageBorderParameters borderParameters);
	[[nodiscard]] GrayscaleFilterResponse CreateSobelYResponse(const Image& grayscaleImage, const ImageBorderParameters borderParameters);
	[[nodiscard]] GrayscaleFilterResponse CreateGradientMagnitudeResponse(
		const GrayscaleFilterResponse& sobelXResponse,
		const GrayscaleFilterResponse& sobelYResponse);

	/** Returns gradient directions as atan2(sobelY, sobelX) in radians within [-pi, pi]. Zero gradients map to 0 radians. */
	[[nodiscard]] GrayscaleFilterResponse CreateGradientDirectionResponse(
		const GrayscaleFilterResponse& sobelXResponse,
		const GrayscaleFilterResponse& sobelYResponse);

	/** Maps finite, non-negative gradient magnitudes from [0, maximum] to [0, 255]. */
	[[nodiscard]] Image CreateNormalizedGradientMagnitudeImage(const GrayscaleFilterResponse& gradientMagnitudeResponse);
}
