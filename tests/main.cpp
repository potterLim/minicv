#include "GrayscaleIntegralImageTest.h"
#include "ImageIoTest.h"
#include "ImageOperationsTest.h"
#include "ImageTest.h"

int main()
{
	RunImageTests();
	RunGrayscaleIntegralImageTests();
	RunImageIoTests();
	RunImageOperationsTests();

	return 0;
}
