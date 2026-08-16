#include <cassert>
#include <vector>

#include "ConvolutionKernelTest.h"
#include "minicv/ConvolutionKernel.h"

namespace
{
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

		(void)kernelSize;
	}
}

void RunConvolutionKernelTests()
{
	TestConvolutionKernelStoresSizeAndCoefficients();
}
