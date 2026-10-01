#include <cassert>
#include <limits>
#include <utility>

#include "GrayscaleFilterResponseTest.h"
#include "minicv/GrayscaleFilterResponse.h"

namespace
{
	void TestMoveSemantics()
	{
		minicv::GrayscaleFilterResponse source(minicv::Size{ 2, 3 });
		source.GetResponseValue(1, 2) = -12.5;
		minicv::GrayscaleFilterResponse moved(std::move(source));
		assert(moved.GetWidth() == 2 && moved.GetHeight() == 3);
		assert(moved.GetResponseValue(1, 2) == -12.5);
		assert(source.IsEmpty());
		assert(source.GetWidth() == 0 && source.GetHeight() == 0);

		minicv::GrayscaleFilterResponse destination(minicv::Size{ 1, 1 });
		destination = std::move(moved);
		assert(destination.GetWidth() == 2 && destination.GetHeight() == 3);
		assert(destination.GetResponseValue(1, 2) == -12.5);
		assert(moved.IsEmpty());
		assert(moved.GetWidth() == 0 && moved.GetHeight() == 0);

		const minicv::GrayscaleFilterResponse emptyMoved(std::move(moved));
		assert(emptyMoved.IsEmpty() && emptyMoved.GetWidth() == 0 && emptyMoved.GetHeight() == 0);
		assert(moved.IsEmpty() && moved.GetWidth() == 0 && moved.GetHeight() == 0);

		minicv::GrayscaleFilterResponse* const alias = &destination;
		destination = std::move(*alias);
		assert(destination.GetWidth() == 2 && destination.GetHeight() == 3);
		assert(destination.GetResponseValue(1, 2) == -12.5);

		const minicv::GrayscaleFilterResponse copied(destination);
		source = copied;
		assert(copied.GetWidth() == 2 && copied.GetHeight() == 3);
		assert(copied.GetResponseValue(1, 2) == -12.5);
		assert(source.GetWidth() == 2 && source.GetHeight() == 3);
		assert(source.GetResponseValue(1, 2) == -12.5);
	}

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

		static_cast<void>(constResponse);
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
	TestMoveSemantics();
	TestGrayscaleFilterResponsePropertiesAndValueAccess();
	TestGrayscaleFilterResponsePreservesEmptySize();
}
