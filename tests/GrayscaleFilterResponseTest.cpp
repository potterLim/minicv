#include <cassert>
#include <limits>

#include "GrayscaleFilterResponseTest.h"
#include "minicv/GrayscaleFilterResponse.h"

namespace
{
	void TestGrayscaleFilterResponsePropertiesAndValueAccess()
	{
		minicv::GrayscaleFilterResponse response(minicv::Size{ 3, 2 });

		assert(!response.IsEmpty());
		assert(response.GetWidth() == 3);
		assert(response.GetHeight() == 2);
		assert(response.GetSize().Width == 3);
		assert(response.GetSize().Height == 2);
		assert(response.GetResponseValue(1, 1) == 0.0);

		response.GetResponseValue(0, 0) = -12.5;
		response.GetResponseValue(2, 1) = 300.25;

		const minicv::GrayscaleFilterResponse& constResponse = response;
		assert(constResponse.GetResponseValue(0, 0) == -12.5);
		assert(constResponse.GetResponseValue(2, 1) == 300.25);

		(void)constResponse;
	}

	void TestGrayscaleFilterResponsePreservesEmptySize()
	{
		const minicv::GrayscaleFilterResponse zeroWidthResponse(minicv::Size{ 0, 5 });
		const minicv::GrayscaleFilterResponse zeroHeightResponse(minicv::Size{ std::numeric_limits<int>::max(), 0 });

		assert(zeroWidthResponse.IsEmpty());
		assert(zeroWidthResponse.GetWidth() == 0);
		assert(zeroWidthResponse.GetHeight() == 5);
		assert(zeroHeightResponse.IsEmpty());
		assert(zeroHeightResponse.GetWidth() == std::numeric_limits<int>::max());
		assert(zeroHeightResponse.GetHeight() == 0);
	}
}

void RunGrayscaleFilterResponseTests()
{
	TestGrayscaleFilterResponsePropertiesAndValueAccess();
	TestGrayscaleFilterResponsePreservesEmptySize();
}
