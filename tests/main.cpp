#include "ImageIoTest.h"
#include "ImageOperationsTest.h"
#include "ImageTest.h"

int main()
{
	RunImageTests();
	RunImageIoTests();
	RunImageOperationsTests();

	return 0;
}
