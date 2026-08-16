#include "ConvolutionKernelTest.h"
#include "GrayscaleIntegralImageTest.h"
#include "ImageFilteringTest.h"
#include "ImageIoTest.h"
#include "ImageOperationsTest.h"
#include "ImageTest.h"

int main()
{
	RunConvolutionKernelTests();
	RunImageTests();
	RunGrayscaleIntegralImageTests();
	RunImageFilteringTests();
	RunImageIoTests();
	RunImageOperationsTests();

	return 0;
}
