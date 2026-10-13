#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "ImageBorder.h"
#include "minicv/ImageGeometry.h"

namespace minicv
{
	namespace
	{
		void CopyPixel(
			const Image& source,
			const int sourceX,
			const int sourceY,
			Image* outDestination,
			const int destinationX,
			const int destinationY)
		{
			assert(outDestination != nullptr);
			assert(source.GetImageType() == outDestination->GetImageType());
			const std::size_t sourceByteIndex = detail::CalculatePixelByteIndex(source, sourceX, sourceY, 0);
			const std::size_t destinationByteIndex = detail::CalculatePixelByteIndex(*outDestination, destinationX, destinationY, 0);
			std::copy_n(source.GetPixelData() + sourceByteIndex, source.GetChannelCount(), outDestination->GetPixelData() + destinationByteIndex);
		}

		int CalculateNearestSourceCoordinate(const int position, const int sourceLength, const int targetLength)
		{
			// Integer scaling preserves floor exactly and avoids overflow in the coordinate product.
			const std::int64_t scaledPosition = static_cast<std::int64_t>(position) * sourceLength;
			return static_cast<int>(scaledPosition / targetLength);
		}

		double CalculateBilinearSourceCoordinate(const int position, const int sourceLength, const int targetLength)
		{
			const double sourcePosition = (static_cast<double>(position) + 0.5) * sourceLength / targetLength - 0.5;
			return std::clamp(sourcePosition, 0.0, static_cast<double>(sourceLength - 1));
		}

		void ResizeNearest(const Image& image, Image* outResizedImage)
		{
			assert(outResizedImage != nullptr);
			for (int y = 0; y < outResizedImage->GetHeight(); ++y)
			{
				const int sourceY = CalculateNearestSourceCoordinate(y, image.GetHeight(), outResizedImage->GetHeight());
				for (int x = 0; x < outResizedImage->GetWidth(); ++x)
				{
					const int sourceX = CalculateNearestSourceCoordinate(x, image.GetWidth(), outResizedImage->GetWidth());
					CopyPixel(image, sourceX, sourceY, outResizedImage, x, y);
				}
			}
		}

		void ResizeBilinear(const Image& image, Image* outResizedImage)
		{
			assert(outResizedImage != nullptr);
			const std::uint8_t* const sourcePixels = image.GetPixelData();
			std::uint8_t* const resizedPixels = outResizedImage->GetPixelData();
			const std::size_t channelCount = static_cast<std::size_t>(image.GetChannelCount());

			for (int y = 0; y < outResizedImage->GetHeight(); ++y)
			{
				const double sourceY = CalculateBilinearSourceCoordinate(y, image.GetHeight(), outResizedImage->GetHeight());
				const int top = static_cast<int>(sourceY);
				const int bottom = std::min(top + 1, image.GetHeight() - 1);
				const double verticalWeight = sourceY - top;

				for (int x = 0; x < outResizedImage->GetWidth(); ++x)
				{
					const double sourceX = CalculateBilinearSourceCoordinate(x, image.GetWidth(), outResizedImage->GetWidth());
					const int left = static_cast<int>(sourceX);
					const int right = std::min(left + 1, image.GetWidth() - 1);
					const double horizontalWeight = sourceX - left;
					const std::size_t topLeftByteIndex = detail::CalculatePixelByteIndex(image, left, top, 0);
					const std::size_t topRightByteIndex = detail::CalculatePixelByteIndex(image, right, top, 0);
					const std::size_t bottomLeftByteIndex = detail::CalculatePixelByteIndex(image, left, bottom, 0);
					const std::size_t bottomRightByteIndex = detail::CalculatePixelByteIndex(image, right, bottom, 0);
					const std::size_t destinationByteIndex = detail::CalculatePixelByteIndex(*outResizedImage, x, y, 0);

					for (std::size_t channelIndex = 0; channelIndex < channelCount; ++channelIndex)
					{
						const double topValue = std::lerp(sourcePixels[topLeftByteIndex + channelIndex], sourcePixels[topRightByteIndex + channelIndex], horizontalWeight);
						const double bottomValue = std::lerp(sourcePixels[bottomLeftByteIndex + channelIndex], sourcePixels[bottomRightByteIndex + channelIndex], horizontalWeight);
						const double pixelValue = std::lerp(topValue, bottomValue, verticalWeight);
						resizedPixels[destinationByteIndex + channelIndex] = static_cast<std::uint8_t>(std::lround(std::clamp(pixelValue, 0.0, 255.0)));
					}
				}
			}
		}
	}

	Image CreateResizedImage(const Image& image, const Size targetSize, const EInterpolationType interpolationType)
	{
		assert(targetSize.Width >= 0 && targetSize.Height >= 0 && "target dimensions must not be negative.");
		const bool isInterpolationSupported = interpolationType == EInterpolationType::NEAREST_NEIGHBOR || interpolationType == EInterpolationType::BILINEAR;
		assert(isInterpolationSupported && "unsupported interpolation type.");
		static_cast<void>(isInterpolationSupported);
		assert((targetSize.Width == 0 || targetSize.Height == 0 || !image.IsEmpty()) && "a nonempty target requires a nonempty source.");

		Image resizedImage(targetSize, image.GetImageType());
		if (resizedImage.IsEmpty())
		{
			return resizedImage;
		}

		switch (interpolationType)
		{
		case EInterpolationType::NEAREST_NEIGHBOR:
			ResizeNearest(image, &resizedImage);
			break;
		case EInterpolationType::BILINEAR:
			ResizeBilinear(image, &resizedImage);
			break;
		default:
			assert(false && "unsupported interpolation type.");
			break;
		}

		return resizedImage;
	}

	Image CreateHorizontallyFlippedImage(const Image& image)
	{
		Image flippedImage(image.GetSize(), image.GetImageType());
		if (image.IsEmpty())
		{
			return flippedImage;
		}

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				CopyPixel(image, image.GetWidth() - 1 - x, y, &flippedImage, x, y);
			}
		}

		return flippedImage;
	}

	Image CreateVerticallyFlippedImage(const Image& image)
	{
		Image flippedImage(image.GetSize(), image.GetImageType());
		if (image.IsEmpty())
		{
			return flippedImage;
		}

		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				CopyPixel(image, x, image.GetHeight() - 1 - y, &flippedImage, x, y);
			}
		}

		return flippedImage;
	}

	Image CreateRotated90Image(const Image& image, const ERotationDirection rotationDirection)
	{
		const bool isDirectionSupported = rotationDirection == ERotationDirection::CLOCKWISE || rotationDirection == ERotationDirection::COUNTERCLOCKWISE;
		assert(isDirectionSupported && "unsupported rotation direction.");
		static_cast<void>(isDirectionSupported);

		Image rotatedImage(Size{ image.GetHeight(), image.GetWidth() }, image.GetImageType());
		if (image.IsEmpty())
		{
			return rotatedImage;
		}

		const bool isClockwise = rotationDirection == ERotationDirection::CLOCKWISE;
		for (int y = 0; y < image.GetHeight(); ++y)
		{
			for (int x = 0; x < image.GetWidth(); ++x)
			{
				const int destinationX = isClockwise ? image.GetHeight() - 1 - y : y;
				const int destinationY = isClockwise ? x : image.GetWidth() - 1 - x;
				CopyPixel(image, x, y, &rotatedImage, destinationX, destinationY);
			}
		}

		return rotatedImage;
	}
}
