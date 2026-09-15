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
}
