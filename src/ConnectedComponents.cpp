#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

#include "minicv/ConnectedComponents.h"
#include "minicv/Image.h"
#include "minicv/Point.h"

namespace minicv
{
	namespace
	{
		constexpr std::array<Point, 8> NEIGHBOR_OFFSETS{
			Point{ -1, 0 }, Point{ 1, 0 }, Point{ 0, -1 }, Point{ 0, 1 },
			Point{ -1, -1 }, Point{ 1, -1 }, Point{ -1, 1 }, Point{ 1, 1 }
		};

		std::size_t CalculatePixelIndex(const int width, const Point position)
		{
			const std::size_t rowOffset = static_cast<std::size_t>(position.Y) * static_cast<std::size_t>(width);
			return rowOffset + static_cast<std::size_t>(position.X);
		}

		ConnectedComponent AnalyzeComponent(
			const Image& image,
			const EConnectivity connectivity,
			const Point seed,
			const std::size_t label,
			std::vector<std::size_t>* outLabels,
			std::vector<Point>* outPendingPixels)
		{
			assert(outLabels != nullptr && outPendingPixels != nullptr);
			outPendingPixels->clear();
			outPendingPixels->push_back(seed);
			(*outLabels)[CalculatePixelIndex(image.GetWidth(), seed)] = label;

			std::size_t area = 0;
			int minimumX = seed.X;
			int maximumX = seed.X;
			int minimumY = seed.Y;
			int maximumY = seed.Y;
			// Floating-point accumulation avoids overflowing integer coordinate sums.
			long double sumX = 0.0L;
			long double sumY = 0.0L;
			const std::size_t neighborCount = connectivity == EConnectivity::FOUR ? 4 : 8;

			while (!outPendingPixels->empty())
			{
				const Point position = outPendingPixels->back();
				outPendingPixels->pop_back();
				++area;
				minimumX = std::min(minimumX, position.X);
				maximumX = std::max(maximumX, position.X);
				minimumY = std::min(minimumY, position.Y);
				maximumY = std::max(maximumY, position.Y);
				sumX += static_cast<long double>(position.X);
				sumY += static_cast<long double>(position.Y);

				for (std::size_t neighborIndex = 0; neighborIndex < neighborCount; ++neighborIndex)
				{
					const Point offset = NEIGHBOR_OFFSETS[neighborIndex];
					const Point neighbor{ position.X + offset.X, position.Y + offset.Y };
					if (!image.Contains(neighbor))
					{
						continue;
					}

					const std::size_t pixelIndex = CalculatePixelIndex(image.GetWidth(), neighbor);
					if ((*outLabels)[pixelIndex] != 0 || image.GetGrayscalePixel(neighbor.X, neighbor.Y) == 0)
					{
						continue;
					}

					// Mark on discovery so each foreground pixel is queued only once.
					(*outLabels)[pixelIndex] = label;
					outPendingPixels->push_back(neighbor);
				}
			}

			const Rect boundingBox{ minimumX, minimumY, maximumX - minimumX + 1, maximumY - minimumY + 1 };
			const double centroidX = static_cast<double>(sumX / static_cast<long double>(area));
			const double centroidY = static_cast<double>(sumY / static_cast<long double>(area));
			return ConnectedComponent{ label, area, boundingBox, centroidX, centroidY };
		}
	}

	ConnectedComponents::ConnectedComponents(const Image& grayscaleImage, const EConnectivity connectivity)
		: mWidth(grayscaleImage.GetWidth())
		, mHeight(grayscaleImage.GetHeight())
	{
		assert(grayscaleImage.GetImageType() == EImageType::UINT8_GRAYSCALE && "image type must be UINT8_GRAYSCALE.");
		assert((connectivity == EConnectivity::FOUR || connectivity == EConnectivity::EIGHT) && "connectivity must be FOUR or EIGHT.");
		const std::size_t pixelCount = grayscaleImage.GetPixelCount();
		assert(pixelCount <= mLabels.max_size() && "label count exceeds maximum vector size.");
		mLabels.resize(pixelCount, 0);
		if (grayscaleImage.IsEmpty())
		{
			return;
		}

		std::vector<Point> pendingPixels;
		for (int y = 0; y < mHeight; ++y)
		{
			for (int x = 0; x < mWidth; ++x)
			{
				const Point position{ x, y };
				const std::size_t pixelIndex = CalculatePixelIndex(mWidth, position);
				if (mLabels[pixelIndex] != 0 || grayscaleImage.GetGrayscalePixel(x, y) == 0)
				{
					continue;
				}

				assert(mComponents.size() < mComponents.max_size() && "component count exceeds maximum vector size.");
				const std::size_t label = mComponents.size() + 1;
				mComponents.push_back(AnalyzeComponent(grayscaleImage, connectivity, position, label, &mLabels, &pendingPixels));
			}
		}
	}

	bool ConnectedComponents::IsEmpty() const
	{
		return mLabels.empty();
	}

	int ConnectedComponents::GetWidth() const
	{
		return mWidth;
	}

	int ConnectedComponents::GetHeight() const
	{
		return mHeight;
	}

	Size ConnectedComponents::GetSize() const
	{
		return Size{ mWidth, mHeight };
	}

	std::size_t ConnectedComponents::GetLabel(const int x, const int y) const
	{
		assert(x >= 0 && x < mWidth && "x is out of range.");
		assert(y >= 0 && y < mHeight && "y is out of range.");
		return mLabels[CalculatePixelIndex(mWidth, Point{ x, y })];
	}

	std::size_t ConnectedComponents::GetComponentCount() const
	{
		return mComponents.size();
	}

	const std::vector<ConnectedComponent>& ConnectedComponents::GetComponents() const
	{
		return mComponents;
	}
}
