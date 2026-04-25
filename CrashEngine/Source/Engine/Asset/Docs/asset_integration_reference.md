# Asset Integration Reference

| Field | Value |
|---|---|
| Author | Codex |
| Created | 2026-04-26 |
| Updated | 2026-04-26 |
| Version | 1.1 |

## 1. Runtime Entry Points

### 1.1 `FAssetObjectManager`

- `LoadStaticMeshObject(const FString&)` is the runtime mesh entry point.
- `LoadMaterialObject(const FString&)` is the runtime imported material entry point.
- `LoadTextureObject(const FString&)` is the runtime texture entry point.
- Source and cooked paths both converge into `FAssetCacheManager`.

### 1.2 UObject Wrappers

- `UAsset` stores shared asset identity state.
- `UStaticMesh` rebuilds `FStaticMesh` and GPU mesh buffers from cooked mesh data.
- `UMaterial` rebuilds runtime parameter state from cooked material data.
- `UTexture2D` rebuilds D3D11 texture resources from cooked texture data.

## 2. Existing Systems Kept On Purpose

- `MaterialManager` still handles JSON-based engine/editor materials.
- `ResourceManager` still handles font and particle resource tables.

## 3. Removed Legacy Paths

- `ObjManager`
- direct `UTexture2D::LoadFromFile(...)` runtime loading

## 4. Current Gaps

- `FontAtlas` UObject bridge is not connected yet.
- `SubUVAtlas` UObject bridge is not connected yet.
