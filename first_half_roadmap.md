# 첫 반년 개발 로드맵

## 목표

miniCV는 핵심 컴퓨터 비전 알고리즘을 C++로 직접 구현하는 이미지 처리 라이브러리이다.

이 기간에는 이미지 처리의 기반을 직접 쌓는다. 이미지 컨테이너, 파일 입출력, 픽셀 연산, 히스토그램, 임계값 처리, 필터링, 엣지 검출, 모폴로지, 객체 영역 분석까지 구현하며 OpenCV가 제공하는 대표 기능들이 내부적으로 어떻게 동작하는지 이해하는 것을 목표로 한다.

## 진행 방식

이 계획은 주 10시간 정도 꾸준히 개발하는 것을 기준으로 26주 동안 진행한다.

4주를 하나의 묶음으로, 앞의 3주는 새 기능을 구현하고 네 번째 주에는 앞서 만든 기능의 오류를 검증하고 미비한 부분을 보완한다. 이때 작은 예제도 함께 만들어 기능이 실제 처리 흐름에 어떻게 연결되는지 확인한다.

마지막 2주에는 전체 기능을 연결하고 예제와 문서를 정리하며 macOS와 Windows에서 같은 방식으로 빌드되는지 확인한다.

구현 난이도나 설계 문제가 드러나면 세부 일정은 조정한다.

## 개발 방향

- OpenCV는 동작 범위를 정하는 기준으로 사용한다.
- 초기 구현은 8-bit grayscale/RGB 이미지에 집중한다.
- 빈 이미지, 크기 불일치, 잘못된 채널 수, 경계 좌표 같은 실패 상황을 구현 초기에 함께 다룬다.
- 각 기능은 테스트뿐 아니라 작은 예제로 연결해 실제 이미지 처리 흐름 속에서 확인한다.
- 예제는 개별 함수 사용법보다 이미지가 단계적으로 처리되는 과정을 보여주는 데 집중한다.

## 26주 계획

### 1단계: 이미지 기반 구조

| 주차 | 주제 | 핵심 내용 |
| ---: | --- | --- |
| 1 | 이미지 컨테이너 1차 구현 | 이미지 크기, 채널 수, stride, raw data 접근, 픽셀 접근, 빈 이미지 판정을 구현한다. |
| 2 | 이미지 컨테이너 기능 확장 | 깊은 복사, 재생성, 전체 채우기, 크기와 shape 비교, `Size`, `Point`, `Rect`, ROI 검증을 추가한다. |
| 3 | 기본 이미지 유틸리티 | 채우기, 복사, 절대 차이, non-zero 픽셀 수, 이미지 동일성 비교, 최대 픽셀 차이 계산을 정리한다. |
| 4 | 기반 기능 보강과 예제 | 앞선 기능의 오류를 검증하고 미비한 API를 보완한다. `Mat` 사용 흐름을 보여주는 작은 예제를 만든다. |

### 2단계: 이미지 입출력

| 주차 | 주제 | 핵심 내용 |
| ---: | --- | --- |
| 5 | Grayscale 이미지 입출력 | PGM `P2`, `P5` 형식의 저장과 읽기를 구현하고 저장한 이미지를 다시 읽어 비교한다. |
| 6 | RGB 이미지 입출력 | PPM `P3`, `P6` 형식의 저장과 읽기를 구현하고 RGB 채널 순서를 검증한다. |
| 7 | 통합 이미지 입출력 | 확장자에 따라 load/save를 선택하고 Netpbm header를 읽는 공통 parser를 정리한다. |
| 8 | 입출력 보강과 예제 | 파싱 오류와 파일 오류를 보완하고, 저장 후 다시 읽어 비교하는 예제를 만든다. |

### 3단계: 픽셀 연산과 히스토그램

| 주차 | 주제 | 핵심 내용 |
| ---: | --- | --- |
| 9 | 색상 변환과 채널 처리 | RGB/grayscale 변환, RGB/BGR swap, 채널 추출/분리/병합을 구현한다. |
| 10 | 기본 픽셀 연산과 영역 처리 | 반전, 밝기/대비 조정, byte clamp, crop, 영역 복사와 붙여넣기를 구현한다. |
| 11 | 히스토그램과 정규화 | Grayscale histogram, CDF, intensity min/max, min-max normalization을 구현한다. |
| 12 | 픽셀 연산과 히스토그램 보강 | 픽셀 연산과 histogram 결과를 직접 계산할 수 있는 작은 이미지로 검증하고 histogram 예제를 만든다. |

### 4단계: 임계값 처리와 필터링 기초

| 주차 | 주제 | 핵심 내용 |
| ---: | --- | --- |
| 13 | 대비 개선과 임계값 처리 | Contrast stretching, histogram equalization, threshold 계열 연산을 구현한다. |
| 14 | Otsu와 adaptive threshold | Otsu, integral image, rectangular sum, adaptive mean threshold를 구현한다. |
| 15 | Border 처리와 convolution | Constant/replicate border 처리와 2D convolution을 구현하고 작은 kernel로 검증한다. |
| 16 | 임계값 처리와 convolution 보강 | Threshold와 convolution에서 드러난 오류를 보완하고 결과 이미지를 저장하는 예제를 만든다. |

### 5단계: 필터링과 엣지 검출

| 주차 | 주제 | 핵심 내용 |
| ---: | --- | --- |
| 17 | Blur와 선명화 | Box blur, Gaussian blur, Gaussian kernel, sharpen, Laplacian response를 구현한다. |
| 18 | Median filter와 signed response | Median filter를 구현하고 signed response를 이미지로 확인할 수 있게 만든다. |
| 19 | Sobel 엣지 | Sobel X/Y, gradient magnitude, gradient direction, edge map normalization을 구현한다. |
| 20 | 필터링과 엣지 보강 | 필터링과 엣지 결과를 비교하는 예제를 만들고 border와 signed response 처리를 보완한다. |

### 6단계: 모폴로지와 객체 영역 분석

| 주차 | 주제 | 핵심 내용 |
| ---: | --- | --- |
| 21 | 모폴로지 | 구조 요소(structuring element), erosion, dilation, opening, closing을 구현한다. |
| 22 | 객체 영역 분석 | Connected components labeling을 구현하고 각 component의 면적, bounding box, centroid를 계산한다. |
| 23 | 기본 기하 변환 | Nearest/bilinear resize, horizontal/vertical flip, 90도 회전을 구현한다. |
| 24 | 영역 분석 보강과 예제 | Threshold, morphology, connected components를 연결해 이진 이미지에서 객체 후보를 찾는 예제를 만든다. |

### 최종 통합

| 주차 | 주제 | 핵심 내용 |
| ---: | --- | --- |
| 25 | 통합 예제 구성 | 이미지를 읽고 변환한 뒤 threshold, filter, edge, morphology, labeling을 차례로 연결한다. |
| 26 | 최종 품질 점검 | 전체 테스트와 예제를 macOS와 Windows에서 실행하고 API, 파일 배치, 경고, README를 정리한다. |

## 기대 상태

이 로드맵의 목표는 miniCV를 단순한 연습 코드가 아니라, 작은 이미지를 읽고 처리하고 분석하는 흐름을 끝까지 수행할 수 있는 C++ 이미지 처리 라이브러리의 형태로 다듬는 것이다.

- `Mat` 기반 이미지 표현, 픽셀 접근, ROI, 복사와 재생성을 안정적으로 지원한다.
- PGM/PPM 이미지를 직접 읽고 쓰며, 작은 테스트 이미지로 입출력 결과를 검증할 수 있다.
- 색상 변환, 채널 처리, 밝기/대비 조정, crop/copy 같은 기본 이미지 연산을 제공한다.
- 히스토그램, equalization, threshold, Otsu, adaptive threshold를 통해 grayscale 이미지를 분석하고 이진화할 수 있다.
- Integral image와 convolution 기반으로 blur, sharpen, median, Laplacian, Sobel edge를 구현한다.
- 모폴로지와 connected components를 이용해 단순한 객체 영역 분석 흐름을 구성할 수 있다.
- Resize, flip, 90도 rotate 같은 기본 기하 변환을 제공한다.
- 기능별 테스트와 통합 예제를 통해 구현 결과를 재현 가능하게 확인할 수 있다.
