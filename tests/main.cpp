#include "ConvolutionKernelTest.h"
#include "GrayscaleFilterResponseTest.h"
#include "GrayscaleIntegralImageTest.h"
#include "ImageFilteringTest.h"
#include "ImageIoTest.h"
#include "ImageOperationsTest.h"
#include "ImageTest.h"

int main()
{
	RunConvolutionKernelTests();
	RunGrayscaleFilterResponseTests();
	RunImageTests();
	RunGrayscaleIntegralImageTests();
	RunImageFilteringTests();
	RunImageIoTests();
	RunImageOperationsTests();

	return 0;
}
