#include "ImageOperationsTest.h"
#include "ImageTest.h"
#include "PgmImageIoTest.h"
#include "PpmImageIoTest.h"

int main()
{
	RunImageTests();
	RunImageOperationsTests();
	RunPgmImageIoTests();
	RunPpmImageIoTests();

	return 0;
}
