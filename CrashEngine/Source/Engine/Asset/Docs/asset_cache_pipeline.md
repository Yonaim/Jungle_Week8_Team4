# Asset Cache Pipeline

| 구분 | 내용 |
|---|---|
| 최초 작성자 | Codex |
| 최초 작성일 | 2026-04-26 |
| 최근 수정자 | Codex |
| 최근 수정일 | 2026-04-26 |
| 버전 | 1.0 |

## 1. 파이프라인

```text
Source File
  -> SourceCache
  -> Intermediate Parse
  -> IntermediateCache
  -> Cooked Build
  -> CookedCache
  -> Binary Save/Load
```

## 2. Source 단계

- `FSourceLoader`가 파일 크기와 수정 시간을 조회한다.
- `TSourceCache`가 normalized path 기준으로 source record를 보관한다.
- 필요할 때만 content hash를 계산한다.

## 3. Intermediate 단계

- `TextureBuilder`는 디코드된 픽셀 데이터를 intermediate로 만든다.
- `MaterialBuilder`는 `.mtl`을 material library intermediate로 만든다.
- `StaticMeshBuilder`는 `.obj`를 face / vertex 참조 중심 intermediate로 만든다.

## 4. Cooked 단계

- texture는 픽셀 버퍼 중심의 `FTextureCookedData`
- material은 슬롯/텍스처 바인딩 중심의 `FMtlCookedData`
- mesh는 정점 버퍼/인덱스/섹션 중심의 `FObjCookedData`

## 5. 캐시 키

- source cache는 normalized path 기준
- intermediate cache는 파싱 결과 해시 기준
- cooked cache는 intermediate key + build settings 기준

## 6. 현재 주의점

- texture cook은 현재 WIC 기반 CPU decode를 사용한다.
- 기본 런타임 경로에서는 `StaticMesh(.obj)`와 `MaterialLibrary(.mtl)`가 실제로 연결되어 있다.
- `FontAtlas`, `SubUVAtlas`는 직렬화 계층은 남아 있지만 현재 런타임 브리지는 연결하지 않았다.
