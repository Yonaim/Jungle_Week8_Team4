# Asset

> Document version: 1.1
> Last updated: 2026-04-26

## Quick Links
- [Module Overview](./Docs/asset_module_overview.md)
- [Cache And Build Pipeline](./Docs/asset_cache_pipeline.md)
- [Integration Reference](./Docs/asset_integration_reference.md)
- [Conventions And Paths](./Docs/asset_conventions_paths.md)

## Summary

The `Asset` module is now the runtime asset entry point for:

- `StaticMesh` from `.obj` or `.obj.bin`
- `Material` from `*.mtl::MaterialName`
- `Texture` from source image files or cooked texture blobs

## Runtime Architecture

- `UAsset` is the common UObject base for runtime asset wrappers.
- `UStaticMesh`, `UMaterial`, and `UTexture2D` now load from cooked asset data.
- `FAssetObjectManager` is the single runtime manager that owns asset-object lookup and cache reuse.
- `FAssetCacheManager` remains the build/cache backend used by the object manager.

## Migration Notes

- Legacy `ObjManager` has been removed.
- Legacy `UTexture2D::LoadFromFile` direct file loading has been removed.
- `MaterialManager` still exists for JSON-based engine/editor material templates, but imported asset materials now resolve through `FAssetObjectManager`.
- `FontAtlas` and `SubUVAtlas` UObject bridges are not wired yet.
