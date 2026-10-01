#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "ImageBorder.h"
#include "minicv/ImageMorphology.h"

namespace minicv
{
	namespace
	{
		enum class EMorphologyOperation
		{
			EROSION,
			DILATION
		};

		Image CreateMorphologicalImage(
			const Image& image,
			const StructuringElement& structuringElement,
			const ImageBorderParameters borderParameters,
			const EMorphologyOperation operation)
		{
			const bool isBorderTypeSupported = borderParameters.BorderType == EBorderType::CONSTANT || borderParameters.BorderType == EBorderType::REPLICATE;
			assert(isBorderTypeSupported && "border type must be CONSTANT or REPLICATE.");
			static_cast<void>(isBorderTypeSupported);

			Image processedImage(image.GetSize(), image.GetImageType());
			if (image.IsEmpty())
			{
				return processedImage;
			}

			const bool isErosion = operation == EMorphologyOperation::EROSION;
			const std::int64_t offsetDirection = isErosion ? 1 : -1;
			const std::uint8_t initialValue = isErosion ? std::numeric_limits<std::uint8_t>::max() : std::numeric_limits<std::uint8_t>::min();
			const int centerX = structuringElement.GetWidth() / 2;
			const int centerY = structuringElement.GetHeight() / 2;
			const std::size_t channelCount = static_cast<std::size_t>(image.GetChannelCount());
			std::uint8_t* const processedPixels = processedImage.GetPixelData();

			for (int y = 0; y < image.GetHeight(); ++y)
			{
				for (int x = 0; x < image.GetWidth(); ++x)
				{
					for (std::size_t channelIndex = 0; channelIndex < channelCount; ++channelIndex)
					{
						std::uint8_t selectedValue = initialValue;
						for (int maskY = 0; maskY < structuringElement.GetHeight(); ++maskY)
						{
							for (int maskX = 0; maskX < structuringElement.GetWidth(); ++maskX)
							{
								if (!structuringElement.IsActive(maskX, maskY))
								{
									continue;
								}

								const std::int64_t sourceX = static_cast<std::int64_t>(x) + offsetDirection * (maskX - centerX);
								const std::int64_t sourceY = static_cast<std::int64_t>(y) + offsetDirection * (maskY - centerY);
								const std::uint8_t pixelValue = detail::GetBorderedPixelValue(image, sourceX, sourceY, channelIndex, borderParameters);
								selectedValue = isErosion ? std::min(selectedValue, pixelValue) : std::max(selectedValue, pixelValue);
							}
						}

						const std::size_t pixelIndex = detail::CalculatePixelByteIndex(processedImage, x, y, channelIndex);
						processedPixels[pixelIndex] = selectedValue;
					}
				}
			}
			return processedImage;
		}
	}

	Image CreateErodedImage(const Image& image, const StructuringElement& structuringElement, const ImageBorderParameters borderParameters)
	{
		return CreateMorphologicalImage(image, structuringElement, borderParameters, EMorphologyOperation::EROSION);
	}

	Image CreateDilatedImage(const Image& image, const StructuringElement& structuringElement, const ImageBorderParameters borderParameters)
	{
		return CreateMorphologicalImage(image, structuringElement, borderParameters, EMorphologyOperation::DILATION);
	}

	Image CreateOpenedImage(const Image& image, const StructuringElement& structuringElement, const ImageBorderParameters borderParameters)
	{
		const Image erodedImage = CreateErodedImage(image, structuringElement, borderParameters);
		return CreateDilatedImage(erodedImage, structuringElement, borderParameters);
	}

	Image CreateClosedImage(const Image& image, const StructuringElement& structuringElement, const ImageBorderParameters borderParameters)
	{
		const Image dilatedImage = CreateDilatedImage(image, structuringElement, borderParameters);
		return CreateErodedImage(dilatedImage, structuringElement, borderParameters);
	}
}
