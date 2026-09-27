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
