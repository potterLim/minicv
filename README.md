# miniCV

miniCV는 핵심 컴퓨터 비전 알고리즘을 C++로 직접 구현하는 이미지 처리 라이브러리입니다.

이미지 표현과 픽셀 접근부터 시작해 파일 입출력, 픽셀 연산, 필터링, 엣지 검출, 모폴로지, 객체 영역 분석까지 차례로 구현하며 이미지 처리 흐름의 내부 동작을 이해하는 것을 목표로 합니다.

## 방향

- 이미지 타입은 8-bit grayscale/RGB로 제한합니다.
- 외부 의존성을 최소화하고 C++ 표준 라이브러리와 CMake 기반으로 구성합니다.
- 테스트는 명확한 입력과 기대 결과를 기준으로 작성합니다.
- API 사용법보다 이미지가 단계적으로 처리되는 흐름을 보여주는 예제를 중시합니다.
- OpenCV의 대표 기능을 참고하되, 알고리즘과 자료구조는 직접 구현합니다.

## 빌드

필요한 도구:

- CMake 3.23 이상
- C++20 지원 컴파일러
- Ninja

Windows MSVC:

```sh
cmake --preset windows-msvc
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug-tests
```

macOS:

```sh
cmake --preset macos
cmake --build --preset macos-debug
ctest --preset macos-debug-tests
```

## 예제

macOS:

```sh
cmake --build --preset macos-debug --target minicv_image_difference_example
./out/build/macos/Debug/minicv_image_difference_example

cmake --build --preset macos-debug --target minicv_histogram_example
./out/build/macos/Debug/minicv_histogram_example

cmake --build --preset macos-debug --target minicv_image_io_round_trip_example
./out/build/macos/Debug/minicv_image_io_round_trip_example

cmake --build --preset macos-debug --target minicv_threshold_and_convolution_example
./out/build/macos/Debug/minicv_threshold_and_convolution_example

cmake --build --preset macos-debug --target minicv_filtering_and_sobel_example
./out/build/macos/Debug/minicv_filtering_and_sobel_example
```

Windows MSVC:

```sh
cmake --build --preset windows-msvc-debug --target minicv_image_difference_example
.\out\build\windows-msvc\Debug\minicv_image_difference_example.exe

cmake --build --preset windows-msvc-debug --target minicv_histogram_example
.\out\build\windows-msvc\Debug\minicv_histogram_example.exe

cmake --build --preset windows-msvc-debug --target minicv_image_io_round_trip_example
.\out\build\windows-msvc\Debug\minicv_image_io_round_trip_example.exe

cmake --build --preset windows-msvc-debug --target minicv_threshold_and_convolution_example
.\out\build\windows-msvc\Debug\minicv_threshold_and_convolution_example.exe

cmake --build --preset windows-msvc-debug --target minicv_filtering_and_sobel_example
.\out\build\windows-msvc\Debug\minicv_filtering_and_sobel_example.exe
```

### 필터링과 Sobel 비교 예제

`FilteringAndSobelExample`은 경계에 닿는 밝은 영역과 점 잡음이 있는 이미지를 만들고,
constant(0)와 replicate 경계 처리 결과를 비교합니다. 박스/가우시안 블러, 중앙값 필터,
선명화, Laplacian, Sobel X/Y와 gradient magnitude를 저장하며,
가우시안 블러 전후의 Sobel 결과도 비교할 수 있습니다.

결과는 시스템 임시 디렉터리의 `minicv_filtering_and_sobel_example/constant`와
`replicate`에 각각 저장됩니다. 실행 시 실제 경로를 출력하고, 모든 PGM 파일은 저장 후
다시 읽어 픽셀이 일치하는지 검증합니다.

| 파일 | 확인할 내용 |
| --- | --- |
| `source.pgm` | 경계에 닿는 사각형과 밝고 어두운 점 잡음 |
| `box_blurred.pgm`, `gaussian_blurred.pgm` | 평활화 방식에 따른 경계와 잡음의 변화 |
| `median_filtered.pgm` | 고립된 점 잡음 제거 |
| `sharpened.pgm` | 경계 강조와 8-bit 범위 제한 |
| `laplacian_signed.pgm` | Laplacian의 양수·음수 응답 |
| `sobel_x_signed.pgm`, `sobel_y_signed.pgm` | X/Y 방향의 밝기 변화와 부호 |
| `sobel_magnitude.pgm`, `gaussian_sobel_magnitude.pgm` | 가우시안 블러 전후의 엣지 형태 |

Signed response 영상은 0을 회색(128), 음수를 어둡게, 양수를 밝게 표시합니다.
각 영상은 자체 최댓값(부호가 있는 응답은 절댓값의 최댓값)으로 정규화하므로,
서로 다른 영상의 밝기를 원래 응답 크기의 절대 비교에 사용하지 않습니다.

## 이미지 입출력 범위

PGM(P2/P5), PPM(P3/P6)의 Maxval=255 파일을 읽으며, 저장은 바이너리 P5/P6 형식입니다.
읽기에 실패하면 `std::nullopt`, 저장에 실패하면 `false`를 반환합니다.

## 모폴로지

`ImageMorphology.h`는 grayscale/RGB 이미지의 침식, 팽창, 열기, 닫기를 제공합니다.
RGB는 채널별로 독립 처리하며, 이진 영상뿐 아니라 일반 8-bit 밝기 값에도 적용할 수 있습니다.

```cpp
#include "minicv/ImageMorphology.h"

const minicv::StructuringElement element =
    minicv::CreateRectangularStructuringElement(minicv::Size{ 3, 3 });
const minicv::ImageBorderParameters border{ minicv::EBorderType::REPLICATE, 0 };
const minicv::Image openedImage = minicv::CreateOpenedImage(sourceImage, element, border);
const minicv::Image closedImage = minicv::CreateClosedImage(openedImage, element, border);
```

구조 요소는 양의 홀수 크기와 중앙 anchor를 사용합니다. 사각형·십자형 생성 함수를 사용하거나,
`StructuringElement(size, maskValues)`에 행 우선 마스크를 전달할 수 있습니다.
0은 제외, 0 이외의 값은 포함을 뜻하며 활성 위치가 하나 이상 있어야 합니다.
비대칭 마스크는 침식에서 중앙 기준 오프셋을 그대로 사용하고, 팽창에서 반전해 사용합니다.

열기는 침식 후 팽창, 닫기는 팽창 후 침식 순서입니다. `CONSTANT` 경계값은 두 단계 모두에
그대로 적용됩니다. 예를 들어 0을 지정하면 침식 시 이미지 가장자리의 밝은 영역도 줄어들 수 있습니다.
출력은 입력 크기와 타입을 유지하며 입력을 수정하지 않습니다. 빈 입력은 같은 shape의 빈 결과를 반환합니다.
잘못된 구조 요소 크기·마스크, 범위 밖 마스크 좌표, 지원하지 않는 경계 타입은 기존 필터와 동일하게
내부 사전 조건 위반으로 취급하여 Debug `assert`로 검사합니다.

## 연결 영역 분석

`ConnectedComponents`는 grayscale 이미지에서 0을 배경, 나머지 값을 전경으로 취급해
연결된 영역에 라벨을 부여하고 면적·경계 박스·중심점을 계산합니다.

```cpp
#include "minicv/ConnectedComponents.h"
#include "minicv/Image.h"

const minicv::ConnectedComponents components(binaryImage, minicv::EConnectivity::EIGHT);
const std::size_t componentCount = components.GetComponentCount();
const std::vector<minicv::ConnectedComponent>& statistics = components.GetComponents();
```

`FOUR`는 상하좌우, `EIGHT`는 대각선까지 연결합니다. 전경의 밝기 값이 서로 달라도
인접하면 같은 영역에 속합니다. 배경 라벨은 0이고, 전경 라벨은 행 우선으로 처음 발견한
영역부터 1, 2, 3 순서입니다. `GetLabel(x, y)`로 픽셀의 라벨을 조회할 수 있으며,
라벨이 n인 영역의 통계는 `GetComponents()[n - 1]`에 있습니다. 배경 통계는 포함하지 않습니다.

면적은 픽셀 수, 경계 박스는 좌상단 위치와 픽셀 단위 너비·높이입니다.
중심점은 영역에 속한 픽셀 좌표의 산술평균이며 밝기에 따른 가중치를 적용하지 않습니다.
라벨은 `std::size_t`로 저장해 255개를 넘는 영역도 구분합니다.

결과는 입력을 수정하거나 참조하지 않고 자체 보관합니다. 빈 입력은 원래 크기를 유지하며,
배경만 있는 정상 크기의 이미지에서는 `IsEmpty()`가 false이고 영역 개수는 0입니다.
RGB 입력, 지원하지 않는 연결성, 범위 밖 좌표는 Debug `assert`로 검사합니다.

## 빌드 설정

제공된 CMake preset은 `Ninja Multi-Config` generator를 사용하며, 다음 설정을 기본으로 합니다.

- C++ 표준: C++20
- C++ 확장: 비활성화
- compile commands: 생성
- 경고 옵션: 활성화

컴파일러별 경고 옵션은 다음과 같습니다.

```text
MSVC       /W4 /permissive- /Zc:__cplusplus
GCC/Clang  -Wall -Wextra -Wpedantic -Wshadow
```

테스트는 `assert` 기반이므로 Debug preset에서 실행하는 것을 기준으로 합니다.

## 구조

```text
include/minicv/        공개 헤더
src/                   라이브러리 구현
examples/              작은 이미지 처리 흐름 예제
tests/                 테스트
```

## 로드맵

miniCV는 반년 단위로 개발 계획을 세우고 각 기간의 목표에 맞춰 기능을 점진적으로 확장합니다.  
첫 개발 계획은 [첫 반년 개발 로드맵](first_half_roadmap.md)에 정리되어 있습니다.

큰 흐름은 다음과 같습니다.

1. 이미지 기반 구조
2. 이미지 입출력
3. 픽셀 연산과 히스토그램
4. 임계값 처리와 필터링 기초
5. 필터링과 엣지 검출
6. 모폴로지와 객체 영역 분석
7. 통합 예제와 품질 점검
