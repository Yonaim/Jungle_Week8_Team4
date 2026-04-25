# Asset Module Overview

| 구분 | 내용 |
|---|---|
| 최초 작성자 | Codex |
| 최초 작성일 | 2026-04-26 |
| 최근 수정자 | Codex |
| 최근 수정일 | 2026-04-26 |
| 버전 | 1.0 |

## 1. 개요

`Asset` 모듈은 원본 에셋 파일을 읽고, 파싱 결과를 intermediate / cooked 단계로
분리해서 캐싱하는 계층이다.

현재 프로젝트에서는 아래 세 가지 타입을 우선 통합했다.

- Texture
- Material Library (`.mtl`)
- Static Mesh (`.obj`)

## 2. 계층 구조

```text
Asset/
├─ Source/
├─ Cache/
├─ Builder/
├─ Cooked/
├─ Serialization/
└─ Manager/
```

## 3. 역할 분리

- `Source`
  - 파일 존재 여부, 파일 크기, 수정 시간, content hash를 관리한다.
- `Cache`
  - source / intermediate / cooked key를 기준으로 데이터를 재사용한다.
- `Builder`
  - 원본 파싱과 cooked data 생성을 담당한다.
- `Cooked`
  - 런타임 브리지나 binary cache가 소비하는 데이터 형식을 정의한다.
- `Serialization`
  - cooked data를 디스크에 저장/복원한다.
- `Manager`
  - 상위 모듈이 직접 호출하는 진입점이다.

## 4. 현재 프로젝트에서의 의미

기존 프로젝트는 텍스처, 메시, 머티리얼 로딩이 각각 다른 매니저에 흩어져 있었다.
이식 후에는 "원본 파싱과 캐시 생성" 책임을 `Asset` 모듈로 모으고,
기존 런타임 객체(`UStaticMesh`, `UMaterial`, `UTexture2D`)는 cooked 결과를 소비하는 쪽으로 정리했다.
