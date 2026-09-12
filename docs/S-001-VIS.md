# Milestone S-001-VIS: 3D Baseline Simulation Visualization & Virtual Interaction Events

## 1. Primary Purpose
Milestone **S-001-VIS** delivers a real-time, hardware-accelerated 3D engineering visualization application for the existing **S-001** simulation baseline. 

This application provides digital engineering oversight and visual verification of:
* The 50 virtual entities modeled by the deterministic simulation core.
* Linear kinematic trajectories in a 3D coordinate frame.
* Synthetic scenario-driven interaction events.
* Real-time simulation telemetry, metrics, and playback control.

> [!NOTE]
> In strict compliance with project governance: this application is software-only, simulation-only, and displays synthetic data only. It contains no physical guidance, weapon control, launch control, tracking filters, real sensors, or operational logic.

---

## 2. System Architecture & Data Flow

The visualization strictly maintains non-intrusive separation between the simulation mathematics and the rendering layer:

```text
+-------------------------------------------------------------+
|              S-001 Simulation Core (C++20)                  |
|  - SimulationClock (dt = 0.05s)                             |
|  - VirtualTarget (50 entities)                              |
|  - GroundTruthGenerator (Seed = 42, std::mt19937_64)        |
+-------------------------------------------------------------+
                              |
                              | Simulation State (TargetState[])
                              v
+-------------------------------------------------------------+
|                 Visualization Adapter                       |
|  - Ingests SimulationEngine states without modifying logic  |
|  - Manages historical trajectory trails (capped buffer)     |
|  - Simulates scenario-defined VirtualInteraction entities   |
|  - Dispatches interaction events upon threshold trigger     |
+-------------------------------------------------------------+
                              |
            +-----------------+-----------------+
            |                                   |
            v                                   v
+-----------------------+           +-----------------------+
|      3D Renderer      |           |  Dashboard & Controls |
|  - OpenGL 4.6 context |           |  - 2D Ortho UI overlay|
|  - Orbit/Pan/Zoom Cam |           |  - Live metrics card  |
|  - 3D Grid & Axes     |           |  - Selected target box|
|  - 50 Target Meshes   |           |  - Live Event Log     |
|  - Trajectory Trails  |           |  - Interactive Buttons|
|  - Interaction Rings  |           +-----------------------+
+-----------------------+
```

---

## 3. Selected Visualization Technology

* **Graphics API:** Hardware-accelerated **OpenGL 4.6.0** (Intel UHD Graphics driver Build 31.0.101.4255) via standard Win32 WGL context.
* **Libraries:** Standard Windows subsystem libraries (`-lopengl32 -lglu32 -lgdi32 -luser32 -lpsapi`).
* **External Third-Party Dependencies:** **Zero**. No GLFW, SDL, or heavy external UI frameworks were installed, preserving the 8 GB RAM environment.
* **Binary Footprint:** ~1.5 MB statically linked executable.

---

## 4. Build Instructions

### Prerequisites
* Windows 11 64-bit
* CMake 3.20+
* GCC 14+ / 16+ (MinGW-w64 with UCRT)

### Build Commands
```powershell
# Navigate to project root
cd Jordanian_RnD_Simulation_PoC

# Configure CMake
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

# Compile all targets
cmake --build build

# Run unit tests
ctest --test-dir build --output-on-failure

# Launch 3D Visualization Application
.\build\bin\jordanian_sim_vis.exe
```

---

## 5. Controls & User Interactions

| Input | Action |
| :--- | :--- |
| **Mouse Left Drag** | Orbit camera (azimuth / elevation) |
| **Mouse Right Drag** | Pan camera plane (X / Y translation) |
| **Mouse Wheel** | Zoom in / Zoom out |
| **Mouse Left Click (Scene)** | Select virtual target (displays telemetry card) |
| **Mouse Left Click (UI)** | Click buttons: `[ START ]`, `[ PAUSE ]`, `[ RESET ]`, speeds, toggles |
| **Spacebar** | Toggle simulation playback (Start / Pause) |
| **Key `R`** | Reset simulation to $t = 0.0\text{ s}$ initial conditions |
| **Key `T`** | Toggle trajectory trails ON / OFF |
| **Key `G`** | Toggle coordinate grid and axes ON / OFF |
| **Key `E`** | Toggle virtual interaction entities ON / OFF |
| **Keys `1` - `5`** | Select playback speed (`0.25x`, `0.5x`, `1.0x`, `2.0x`, `5.0x`) |
| **Escape** | Exit application cleanly |

---

## 6. Scenario Configuration (`scenarios/S-001_visual_demo.json`)

Extends the S-001 baseline with synthetic scenario-defined interaction entities:

```json
{
  "scenario_id": "S-001-VIS",
  "seed": 42,
  "timestep_seconds": 0.05,
  "duration_seconds": 10.0,
  "target_count": 50,
  "targets": [ ... ],
  "virtual_event_count": 3,
  "virtual_events": [
    {
      "event_entity_id": "VE-001",
      "target_id": "VT-017",
      "spawn_time": 1.0,
      "lifetime": 4.0,
      "event_trigger_threshold": 80.0,
      "event_type": "SIMULATED_INTERACTION",
      "position": { "x": -3512.2, "y": 6363.7, "z": 2185.0 },
      "velocity": { "vx": 285.7, "vy": -214.3, "vz": 71.4 }
    },
    ...
  ]
}
```

---

## 7. Virtual Interaction Model

1. **Entity Representation (`VirtualInteractionEntity`):**
   * Identified by IDs (`VE-001`, `VE-002`, `VE-003`).
   * Rendered as distinct neon-magenta octahedrons with directional velocity vectors.
2. **Deterministic Trigger:**
   * Purely scenario-defined: evaluates Euclidean distance $\Delta d = \|\mathbf{r}_{\text{VE}} - \mathbf{r}_{\text{VT}}\|$.
   * When $\Delta d \le \text{event\_trigger\_threshold}$ ($80.0\text{ m}$):
     * Target state changes from `ACTIVE` to `INTERACTED` (marker color shifts from Cyan to Amber/Orange).
     * Lightweight visual effect generated: expanding ring at interaction coordinates with fading opacity.
     * Structured event recorded in `EventManager` and rendered in the live Event Log.

---

## 8. Test Validation Results

All tests run via `build/bin/jordanian_sim_tests.exe`:

| Test Name | Milestone | Scope | Result |
| :--- | :--- | :--- | :--- |
| `test_target_creation` | S-001 | VirtualTarget model integrity | **PASSED** |
| `test_deterministic_seed_behavior` | S-001 | MT19937-64 pseudo-random reproducibility | **PASSED** |
| `test_simulation_clock_progression` | S-001 | Clock ticks & step integration | **PASSED** |
| `test_target_state_update` | S-001 | Linear kinematic update | **PASSED** |
| `test_50_target_scenario_loading` | S-001 | Baseline JSON ingestion | **PASSED** |
| `test_vis_state_50_targets` | S-001-VIS | 50 targets initialized in adapter | **PASSED** |
| `test_vis_deterministic_replay` | S-001-VIS | Bit-exact state match across repeated runs | **PASSED** |
| `test_vis_virtual_interaction` | S-001-VIS | Event triggers, state changes to INTERACTED | **PASSED** |
| `test_vis_reset` | S-001-VIS | Complete state & log restoration to $t = 0.0\text{ s}$ | **PASSED** |

**Summary:** **9 Passed, 0 Failed.**

---

## 9. Measured Performance vs. TBM Requirements

Measurements obtained using the built-in benchmark harness (`jordanian_sim_vis.exe --benchmark --frames=300`):

| Parameter / Metric | Measured Value | Status | Measurement Method |
| :--- | :--- | :--- | :--- |
| **Startup Time** | `97.65 ms` | **Measured** | High-resolution initialization clock |
| **Average Frame Rate** | `370.0 FPS` | **Measured** | Elapsed wall-clock time over 300 render cycles |
| **Average Frame Time** | `2.703 ms` | **Measured** | Per-frame render + swap interval |
| **Sim Adapter Step Time** | `3.16 us` | **Measured** | Per-step simulation adapter execution |
| **Active Virtual Targets** | `50` | **Measured** | Target collection count |
| **Active Virtual Event Markers** | `2` (concurrent active) | **Measured** | Active entity registry |
| **Triggered Interaction Events** | `1` (at benchmark exit frame) | **Measured** | Event manager count |
| **Process Working Set (Memory)**| `119.89 MB` | **Measured** | `GetProcessMemoryInfo` (includes OpenGL driver) |
| **CPU Usage Percentage** | **TBM** | **TBM** | Requires non-intrusive system profiler |
| **Display Panel VSync Latency** | **TBM** | **TBM** | Dependent on physical monitor refresh rate |
| **Multi-GPU Scalability** | **TBM** | **TBM** | Single Intel UHD GPU utilized |

---

## 10. Known Limitations
1. **Linear Kinematic Assumption:** Virtual entities propagate along constant velocity vectors without aerodynamic drag or gravitational curvature (as specified for Milestone S-001 baseline).
2. **Fixed Trail Buffer:** Trajectory history is capped at 40 points per entity to avoid unbounded memory allocation.
3. **Software-Only Boundary:** The visual ring effect is a synthetic indicator, not a physical hydrodynamic or thermodynamic expansion model.
