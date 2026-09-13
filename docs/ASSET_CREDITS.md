# S-001-VIS-ASSETS Asset Credits

All 3D assets in this simulation are procedurally generated using legacy OpenGL immediate mode.
No external model files (GLB, GLTF, OBJ, FBX) are used.

## Procedural Meshes

| Asset Type | File | Description | Approx. Vertex Count |
|---|---|---|---|
| RADAR_NODE | `src/visualization/models/RadarNodeMesh.hpp` | Parabolic dish on vertical pole with cross-arm base. 8-sided cylinder, 12-segment dish, 8-segment radar bowl, 6-face box. | ~320 |
| CONTROL_CENTER | `src/visualization/models/ControlCenterMesh.hpp` | Hexagonal main building with roof slab, vertical antenna tower, 10-segment radome sphere, 12-segment parabolic antenna, flat platform base. | ~400 |
| LAUNCHER_PLATFORM | `src/visualization/models/LauncherPlatformMesh.hpp` | Rectangular chassis with 4 wheel cylinders, turret ring, primary barrel, secondary barrel. | ~360 |
| EMITTER_POINT | `src/visualization/AssetManager.cpp` | 8-segment octahedral emitter shape with two opposing cones. | ~24 |
| PROJECTILE | `src/visualization/AssetManager.cpp` | 6-segment pointed cone with base fins. | ~36 |
| ENVIRONMENT_GROUND | `src/visualization/Renderer.cpp` | Ground plane quad + horizon rings. | N/A (immediate mode) |

## Color Palette

| Asset | Primary Color | RGB |
|---|---|---|
| Radar dish | Steel blue | (80, 180, 220) |
| Radar pole | Dark gray | (90, 95, 100) |
| Control center walls | Charcoal | (70, 75, 85) |
| Control center roof | Dark slate | (55, 60, 70) |
| Radome | Light blue-gray | (160, 180, 200) |
| Launcher chassis | Gray metal | (85, 90, 95) |
| Launcher turret | Medium gray | (100, 110, 120) |
| Launcher barrel | Dark gray | (70, 75, 80) |
| Wheels | Near-black | (40, 42, 45) |
| Emitter point | Magenta | (200, 60, 140) |
| Projectile | Gold | (255, 180, 40) |

## Attribution

All procedural meshes are original code developed for this project.
No third-party model libraries or external asset sources are used.

## License

All assets in this repository are covered by the same license as the project source code.
