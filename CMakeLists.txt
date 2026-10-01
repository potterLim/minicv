cmake_minimum_required(VERSION 3.23)

project(miniCV
    VERSION 0.1.0
    DESCRIPTION "Core computer vision algorithms implemented from scratch in C++."
    LANGUAGES CXX
)

add_library(miniCV STATIC
    src/ConnectedComponents.cpp
    src/ConvolutionKernel.cpp
    src/GrayscaleFilterResponse.cpp
    src/GrayscaleIntegralImage.cpp
    src/Image.cpp
    src/ImageBorder.cpp
    src/ImageBorder.h
    src/ImageFiltering.cpp
    src/ImageIo.cpp
    src/ImageMorphology.cpp
    src/ImageOperations.cpp
    src/NetpbmImageIo.cpp
    src/NetpbmImageIo.h
    src/PgmImageIo.cpp
    src/PgmImageIo.h
    src/PpmImageIo.cpp
    src/PpmImageIo.h
    src/StructuringElement.cpp
    include/minicv/ConvolutionKernel.h
    include/minicv/ConnectedComponent.h
    include/minicv/ConnectedComponents.h
    include/minicv/EConnectivity.h
    include/minicv/Image.h
    include/minicv/ImageBorderParameters.h
    include/minicv/ImageFiltering.h
    include/minicv/ImageIo.h
    include/minicv/ImageMorphology.h
    include/minicv/ImageOperations.h
    include/minicv/EBorderType.h
    include/minicv/EImageType.h
    include/minicv/ERgbChannel.h
    include/minicv/EThresholdType.h
    include/minicv/GrayscaleAdaptiveThresholdParameters.h
    include/minicv/GrayscaleCumulativeDistribution.h
    include/minicv/GrayscaleFilterResponse.h
    include/minicv/GrayscaleHistogram.h
    include/minicv/GrayscaleIntegralImage.h
    include/minicv/GrayscaleThresholdParameters.h
    include/minicv/GrayscaleValueRange.h
    include/minicv/Point.h
    include/minicv/Rect.h
    include/minicv/Size.h
    include/minicv/StructuringElement.h
)
add_library(miniCV::miniCV ALIAS miniCV)

target_compile_features(miniCV
    PUBLIC
        cxx_std_20
)

target_include_directories(miniCV
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

option(MINICV_ENABLE_WARNINGS "Enable compiler warnings for miniCV." ON)
option(MINICV_BUILD_TESTS "Build miniCV tests." ${PROJECT_IS_TOP_LEVEL})
option(MINICV_BUILD_EXAMPLES "Build miniCV examples." ${PROJECT_IS_TOP_LEVEL})

function(minicv_enable_warnings target_name)
    if(MINICV_ENABLE_WARNINGS)
        target_compile_options(${target_name}
            PRIVATE
                $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /Zc:__cplusplus>
                $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wshadow>
        )
    endif()
endfunction()

minicv_enable_warnings(miniCV)

if(MINICV_BUILD_TESTS)
    include(CTest)

    if(BUILD_TESTING)
        add_executable(minicv_tests
            tests/main.cpp
            tests/ConnectedComponentsTest.cpp
            tests/ConnectedComponentsTest.h
            tests/ConvolutionKernelTest.cpp
            tests/ConvolutionKernelTest.h
            tests/GrayscaleFilterResponseTest.cpp
            tests/GrayscaleFilterResponseTest.h
            tests/GrayscaleIntegralImageTest.cpp
            tests/GrayscaleIntegralImageTest.h
            tests/ImageFilteringTest.cpp
            tests/ImageFilteringTest.h
            tests/ImageIoTest.cpp
            tests/ImageIoTest.h
            tests/ImageMorphologyTest.cpp
            tests/ImageMorphologyTest.h
            tests/ImageOperationsTest.cpp
            tests/ImageOperationsTest.h
            tests/ImageTest.cpp
            tests/ImageTest.h
            tests/StructuringElementTest.cpp
            tests/StructuringElementTest.h
        )

        target_link_libraries(minicv_tests
            PRIVATE
                miniCV::miniCV
        )

        minicv_enable_warnings(minicv_tests)

        # Isolate allocation fault injection from the regular test executable.
        add_executable(minicv_copy_assignment_tests tests/CopyAssignmentTest.cpp)
        target_link_libraries(minicv_copy_assignment_tests PRIVATE miniCV::miniCV)
        minicv_enable_warnings(minicv_copy_assignment_tests)

        # Test checks must execute in every configuration, including Release.
        foreach(test_target IN ITEMS minicv_tests minicv_copy_assignment_tests)
            target_compile_options(${test_target} PRIVATE
                $<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>
                $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-UNDEBUG>
            )
        endforeach()
        add_test(NAME minicv.copy_assignment COMMAND minicv_copy_assignment_tests)

        add_test(
            NAME minicv.tests
            COMMAND minicv_tests
        )
    endif()
endif()

if(MINICV_BUILD_EXAMPLES)
    add_executable(minicv_image_difference_example
        examples/ImageDifferenceExample.cpp
    )

    add_executable(minicv_histogram_example
        examples/HistogramExample.cpp
    )

    add_executable(minicv_image_io_round_trip_example
        examples/ImageIoRoundTripExample.cpp
    )

    add_executable(minicv_threshold_and_convolution_example
        examples/ThresholdAndConvolutionExample.cpp
    )

    add_executable(minicv_filtering_and_sobel_example
        examples/FilteringAndSobelExample.cpp
    )

    target_link_libraries(minicv_image_difference_example
        PRIVATE
            miniCV::miniCV
    )

    target_link_libraries(minicv_histogram_example
        PRIVATE
            miniCV::miniCV
    )

    target_link_libraries(minicv_image_io_round_trip_example
        PRIVATE
            miniCV::miniCV
    )

    target_link_libraries(minicv_threshold_and_convolution_example
        PRIVATE
            miniCV::miniCV
    )

    target_link_libraries(minicv_filtering_and_sobel_example
        PRIVATE
            miniCV::miniCV
    )

    minicv_enable_warnings(minicv_image_difference_example)
    minicv_enable_warnings(minicv_histogram_example)
    minicv_enable_warnings(minicv_image_io_round_trip_example)
    minicv_enable_warnings(minicv_threshold_and_convolution_example)
    minicv_enable_warnings(minicv_filtering_and_sobel_example)
endif()
