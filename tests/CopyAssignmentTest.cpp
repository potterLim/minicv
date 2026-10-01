#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <vector>

#include "minicv/ConnectedComponents.h"
#include "minicv/ConvolutionKernel.h"
#include "minicv/GrayscaleFilterResponse.h"
#include "minicv/GrayscaleIntegralImage.h"
#include "minicv/Image.h"
#include "minicv/StructuringElement.h"

#ifdef NDEBUG
#error Test assertions must remain enabled in every build configuration.
#endif

namespace
{
	// This single-threaded executable replaces scalar allocation for vector storage.
	int allocationsUntilFailure = 0;

	class AllocationFailureScope
	{
	public:
		explicit AllocationFailureScope(const int allocationNumber)
		{
			assert(allocationNumber > 0 && allocationsUntilFailure == 0);
			allocationsUntilFailure = allocationNumber;
		}

		~AllocationFailureScope()
		{
			allocationsUntilFailure = 0;
		}

		AllocationFailureScope(const AllocationFailureScope&) = delete;
		AllocationFailureScope& operator=(const AllocationFailureScope&) = delete;
	};

	void AssertSame(const minicv::Image& actual, const minicv::Image& expected)
	{
		assert(actual.HasSameContent(expected));
		assert(actual.GetByteCount() == expected.GetByteCount());
		assert(actual.GetBytesPerRow() == expected.GetBytesPerRow());
	}

	void AssertSame(const minicv::GrayscaleFilterResponse& actual, const minicv::GrayscaleFilterResponse& expected)
	{
		assert(actual.GetWidth() == expected.GetWidth() && actual.GetHeight() == expected.GetHeight());
		for (int y = 0; y < expected.GetHeight(); ++y)
		{
			for (int x = 0; x < expected.GetWidth(); ++x)
			{
				assert(actual.GetResponseValue(x, y) == expected.GetResponseValue(x, y));
			}
		}
	}

	void AssertSame(const minicv::GrayscaleIntegralImage& actual, const minicv::GrayscaleIntegralImage& expected)
	{
		assert(actual.GetWidth() == expected.GetWidth() && actual.GetHeight() == expected.GetHeight());
		for (int y = 0; y < expected.GetHeight(); ++y)
		{
			for (int x = 0; x < expected.GetWidth(); ++x)
			{
				const minicv::Rect region{ 0, 0, x + 1, y + 1 };
				assert(actual.GetRegionSum(region) == expected.GetRegionSum(region));
			}
		}
	}

	void AssertSame(const minicv::ConvolutionKernel& actual, const minicv::ConvolutionKernel& expected)
	{
		assert(actual.GetWidth() == expected.GetWidth() && actual.GetHeight() == expected.GetHeight());
		for (int y = 0; y < expected.GetHeight(); ++y)
		{
			for (int x = 0; x < expected.GetWidth(); ++x)
			{
				assert(actual.GetCoefficient(x, y) == expected.GetCoefficient(x, y));
			}
		}
	}

	void AssertSame(const minicv::StructuringElement& actual, const minicv::StructuringElement& expected)
	{
		assert(actual.GetWidth() == expected.GetWidth() && actual.GetHeight() == expected.GetHeight());
		for (int y = 0; y < expected.GetHeight(); ++y)
		{
			for (int x = 0; x < expected.GetWidth(); ++x)
			{
				assert(actual.IsActive(x, y) == expected.IsActive(x, y));
			}
		}
	}

	void AssertSame(const minicv::ConnectedComponents& actual, const minicv::ConnectedComponents& expected)
	{
		assert(actual.GetWidth() == expected.GetWidth() && actual.GetHeight() == expected.GetHeight());
		assert(actual.GetComponentCount() == expected.GetComponentCount());
		for (int y = 0; y < expected.GetHeight(); ++y)
		{
			for (int x = 0; x < expected.GetWidth(); ++x)
			{
				assert(actual.GetLabel(x, y) == expected.GetLabel(x, y));
			}
		}
		for (std::size_t index = 0; index < expected.GetComponentCount(); ++index)
		{
			const minicv::ConnectedComponent& left = actual.GetComponents()[index];
			const minicv::ConnectedComponent& right = expected.GetComponents()[index];
			assert(left.Label == right.Label && left.Area == right.Area);
			assert(left.BoundingBox.X == right.BoundingBox.X && left.BoundingBox.Y == right.BoundingBox.Y);
			assert(left.BoundingBox.Width == right.BoundingBox.Width && left.BoundingBox.Height == right.BoundingBox.Height);
			assert(left.CentroidX == right.CentroidX && left.CentroidY == right.CentroidY);
		}
	}

	template <typename T>
	void TestCopyAssignment(const T& source, const T& initialValue, const int allocationCount)
	{
		const T originalSource(source);
		for (int allocationNumber = 1; allocationNumber <= allocationCount; ++allocationNumber)
		{
			T destination(initialValue);
			bool didThrow = false;
			{
				const AllocationFailureScope failure(allocationNumber);
				try
				{
					destination = source;
				}
				catch (const std::bad_alloc&)
				{
					didThrow = true;
				}
			}
			assert(didThrow);
			AssertSame(destination, initialValue);
			AssertSame(source, originalSource);

			// A failed assignment must leave the destination reusable.
			destination = source;
			AssertSame(destination, source);
			{
				const AllocationFailureScope failure(1);
				const T* const alias = &destination;
				destination = *alias;
			}
			AssertSame(destination, source);
		}
	}
}

void* operator new(const std::size_t size)
{
	if (allocationsUntilFailure > 0 && --allocationsUntilFailure == 0)
	{
		throw std::bad_alloc();
	}
	void* const memory = std::malloc(size == 0 ? 1 : size);
	if (memory == nullptr)
	{
		throw std::bad_alloc();
	}
	return memory;
}

void operator delete(void* memory) noexcept
{
	std::free(memory);
}

void operator delete(void* memory, std::size_t) noexcept
{
	std::free(memory);
}

int main()
{
	minicv::Image source(3, 3);
	source.Fill(10);
	source.GetGrayscalePixel(1, 1) = 0;
	minicv::Image initialImage(1, 1, minicv::EImageType::UINT8_RGB);
	initialImage.FillRgb(7, 8, 9);
	TestCopyAssignment(source, initialImage, 1);
	TestCopyAssignment(source, minicv::Image{}, 1);

	minicv::GrayscaleFilterResponse response(minicv::Size{ 3, 3 });
	response.GetResponseValue(2, 2) = -12.5;
	minicv::GrayscaleFilterResponse initialResponse(minicv::Size{ 1, 1 });
	initialResponse.GetResponseValue(0, 0) = 3.5;
	TestCopyAssignment(response, initialResponse, 1);
	TestCopyAssignment(minicv::GrayscaleIntegralImage(source), minicv::GrayscaleIntegralImage(minicv::Image(1, 1)), 1);
	TestCopyAssignment(minicv::ConvolutionKernel({ 3, 1 }, { -1.0, 0.0, 2.0 }), minicv::ConvolutionKernel({ 1, 1 }, { 4.0 }), 1);
	TestCopyAssignment(minicv::StructuringElement({ 3, 1 }, { 0, 1, 0 }), minicv::StructuringElement({ 1, 1 }, { 1 }), 1);

	// Fail both the label allocation and the subsequent component-statistics allocation.
	TestCopyAssignment(
		minicv::ConnectedComponents(source, minicv::EConnectivity::FOUR),
		minicv::ConnectedComponents(minicv::Image(1, 1), minicv::EConnectivity::FOUR), 2);
	return 0;
}
