#pragma once

#include "minicv/Image.h"
#include "minicv/ImageBorderParameters.h"

namespace minicv
{
	class ConvolutionKernel;

	[[nodiscard]] Image CreateConvolvedImage(const Image& image, const ConvolutionKernel& kernel, const ImageBorderParameters borderParameters);
}
