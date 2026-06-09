#include "ImageOperationsTest.h"
#include "ImageTest.h"
#include "PgmImageIoTest.h"

int main()
{
	RunImageTests();
	RunImageOperationsTests();
	RunPgmImageIoTests();

	return 0;
}
