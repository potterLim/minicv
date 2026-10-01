#include <cassert>
#include <utility>
#include <vector>

#include "ConvolutionKernelTest.h"
#include "minicv/ConvolutionKernel.h"

namespace
{
	void TestMoveSemantics()
	{
		minicv::ConvolutionKernel source(minicv::Size{ 3, 1 }, { -1.0, 0.0, 2.0 });
		minicv::ConvolutionKernel moved(std::move(source));
		assert(moved.GetWidth() == 3 && moved.GetHeight() == 1);
		assert(moved.GetCoefficient(0, 0) == -1.0);
		assert(moved.GetCoefficient(2, 0) == 2.0);
		assert(source.GetWidth() == 1 && source.GetHeight() == 1);
		assert(source.GetCoefficient(0, 0) == 1.0);

		minicv::ConvolutionKernel destination(minicv::Size{ 1, 1 }, { 3.0 });
		destination = std::move(moved);
		assert(destination.GetWidth() == 3 && destination.GetHeight() == 1);
		assert(destination.GetCoefficient(0, 0) == -1.0);
		assert(destination.GetCoefficient(2, 0) == 2.0);
		assert(moved.GetWidth() == 1 && moved.GetHeight() == 1);
		assert(moved.GetCoefficient(0, 0) == 3.0);

		minicv::ConvolutionKernel* const alias = &destination;
		destination = std::move(*alias);
		assert(destination.GetWidth() == 3 && destination.GetHeight() == 1);
		assert(destination.GetCoefficient(0, 0) == -1.0);
		assert(destination.GetCoefficient(2, 0) == 2.0);

		const minicv::ConvolutionKernel copied(destination);
		source = copied;
		assert(copied.GetWidth() == 3 && copied.GetHeight() == 1);
		assert(copied.GetCoefficient(0, 0) == -1.0);
		assert(copied.GetCoefficient(2, 0) == 2.0);
		assert(source.GetWidth() == 3 && source.GetHeight() == 1);
		assert(source.GetCoefficient(0, 0) == -1.0);
		assert(source.GetCoefficient(2, 0) == 2.0);
	}

	void TestConvolutionKernelStoresSizeAndCoefficients()
	{
		const std::vector<double> coefficients{
			1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0
		};
		const minicv::ConvolutionKernel kernel(minicv::Size{ 3, 3 }, coefficients);

		assert(kernel.GetWidth() == 3);
		assert(kernel.GetHeight() == 3);

		const minicv::Size kernelSize = kernel.GetSize();
		assert(kernelSize.Width == 3);
		assert(kernelSize.Height == 3);
		assert(kernel.GetCoefficient(0, 0) == 1.0);
		assert(kernel.GetCoefficient(2, 0) == 3.0);
		assert(kernel.GetCoefficient(1, 1) == 5.0);
		assert(kernel.GetCoefficient(0, 2) == 7.0);
		assert(kernel.GetCoefficient(2, 2) == 9.0);

		static_cast<void>(kernelSize);
	}
}

void RunConvolutionKernelTests()
{
	TestMoveSemantics();
	TestConvolutionKernelStoresSizeAndCoefficients();
}
