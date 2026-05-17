# miniCV

miniCV는 핵심 컴퓨터 비전 알고리즘을 C++로 직접 구현하는 이미지 처리 라이브러리입니다.

이미지 표현, 픽셀 접근, 파일 입출력, 픽셀 연산, 필터링, 엣지 검출, 모폴로지, 객체 영역 분석을 차례로 구현하며 이미지 처리 흐름의 내부 동작을 이해하는 것을 목표로 합니다.

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

## 빌드 설정

제공된 CMake preset은 `Ninja Multi-Config` generator를 사용하며, 다음 설정을 기본으로 합니다.

- C++ 표준: C++20
- C++ 확장: 비활성화
- compile commands: 생성
- 경고 옵션: 활성화

컴파일러별 경고 옵션은 다음과 같습니다.

```text
MSVC       /W4 /permissive- /Zc:__cplusplus
GCC/Clang  -Wall -Wextra -Wpedantic
```

테스트는 `assert` 기반이므로 Debug preset에서 실행하는 것을 기준으로 합니다.

## 구조

```text
include/minicv/        공개 헤더
src/                   라이브러리 구현
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
