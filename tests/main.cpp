#include "ConnectedComponentsTest.h"
#include "ConvolutionKernelTest.h"
#include "GrayscaleFilterResponseTest.h"
#include "GrayscaleIntegralImageTest.h"
#include "ImageFilteringTest.h"
#include "ImageIoTest.h"
#include "ImageMorphologyTest.h"
#include "ImageOperationsTest.h"
#include "ImageTest.h"
#include "StructuringElementTest.h"

int main()
{
	RunConnectedComponentsTests();
	RunConvolutionKernelTests();
	RunGrayscaleFilterResponseTests();
	RunImageTests();
	RunGrayscaleIntegralImageTests();
	RunImageFilteringTests();
	RunImageIoTests();
	RunImageOperationsTests();
	RunStructuringElementTests();
	RunImageMorphologyTests();

	return 0;
}
