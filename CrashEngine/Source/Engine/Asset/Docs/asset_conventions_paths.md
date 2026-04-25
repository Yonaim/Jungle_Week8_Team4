# Asset Conventions: Paths

| 구분 | 내용 |
|---|---|
| 최초 작성자 | Codex |
| 최초 작성일 | 2026-04-26 |
| 최근 수정자 | Codex |
| 최근 수정일 | 2026-04-26 |
| 버전 | 1.0 |

## 1. 기본 원칙

- cache key는 가능한 한 absolute normalized path 기준으로 만든다.
- 외부에 노출하는 문자열 경로는 UTF-8 문자열 경로를 사용한다.
- source path와 cooked path를 파일 확장자로 구분한다.

## 2. 주요 확장자

| 타입 | source | cooked |
|---|---|---|
| Texture | `.png`, `.jpg`, `.jpeg`, `.bmp` | `.texture` |
| Material Library | `.mtl` | `.mtl.bin` |
| Static Mesh | `.obj` | `.obj.bin` |

## 3. 머티리얼 경로 규칙

- material library 내부 개별 머티리얼은 `LibraryPath::MaterialName` 형식을 사용한다.
- 예: `Asset/Content/Models/Dice/Dice.mtl::Material`

## 4. 메시 경로 규칙

- 기본 source는 `.obj`
- cooked mesh는 동일 경로 기반 `.obj.bin`
- editor asset 목록에서 cooked mesh는 `*.obj.bin` 파일 자체를 노출한다.

## 5. 런타임 변환 규칙

- `FPaths::Normalize(...)`로 상대 경로를 프로젝트 루트 기준 절대 경로로 정규화한다.
- `MaterialManager`는 `::` 구분자가 있는 경로일 때 library path 부분만 정규화한다.
