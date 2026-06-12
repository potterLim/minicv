cmake_minimum_required(VERSION 3.23)

project(miniCV
    VERSION 0.1.0
    DESCRIPTION "Core computer vision algorithms implemented from scratch in C++."
    LANGUAGES CXX
)

add_library(miniCV STATIC
    src/Image.cpp
    src/ImageOperations.cpp
    src/PgmImageIo.cpp
    src/PpmImageIo.cpp
    include/minicv/Image.h
    include/minicv/ImageOperations.h
    include/minicv/PgmImageIo.h
    include/minicv/PpmImageIo.h
    include/minicv/EImageType.h
    include/minicv/EPgmFormat.h
    include/minicv/EPpmFormat.h
    include/minicv/ERgbChannel.h
    include/minicv/Point.h
    include/minicv/Rect.h
    include/minicv/Size.h
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
            tests/ImageOperationsTest.cpp
            tests/ImageOperationsTest.h
            tests/ImageTest.cpp
            tests/ImageTest.h
            tests/PgmImageIoTest.cpp
            tests/PgmImageIoTest.h
            tests/PpmImageIoTest.cpp
            tests/PpmImageIoTest.h
        )

        target_link_libraries(minicv_tests
            PRIVATE
                miniCV::miniCV
        )

        minicv_enable_warnings(minicv_tests)

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

    target_link_libraries(minicv_image_difference_example
        PRIVATE
            miniCV::miniCV
    )

    minicv_enable_warnings(minicv_image_difference_example)
endif()
