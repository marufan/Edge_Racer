# EDGE RACER

A fast-paced, high-momentum first-person parkour endless runner inspired by **Mirror's Edge**, built from scratch in **Modern C++20** and **Raylib 5.0**.

---

## 🏃 Controls

| Action | Key / Input | Description |
| :--- | :--- | :--- |
| **Look Around** | Mouse | First-person pitch & yaw look (cursor auto-locked) |
| **Strafe** | `A` / `D` or `Left` / `Right` | Lateral lane adjustment with camera lean |
| **Jump** | `Space` | Jump across rooftops, jump buffering & coyote time supported |
| **Vault / Mantle** | `Space` (near ledge) | Smoothly vault over waist/chest-high obstacles |
| **Power Slide** | `Left Ctrl`, `C`, or `Right Click` | Duck low (camera drops to 0.7m) to slide under low red pipes |
| **Slide Kick** | `Space` during a slide | High-speed launch jump out of a slide |
| **Wall-Run** | Auto (in air near red billboard) | Glides along walls with 15° camera tilt & reduced gravity |
| **Wall Kick** | `Space` during a wall-run | Explosive outward + forward leap with bonus momentum |
| **Skill Roll** | Hold `Ctrl` / `C` / `Right Click` before hard landing | Roll upon ground impact, eliminating stumble damage and boosting speed |
| **Springboards** | Run onto red ramps | Automatic massive parabolic launch across wide skyscraper gaps |
| **Restart** | `R` or `Space` (after crash) | Instant quick-restart |
| **Pause** | `P` | Pause and unpause the game |

---

## 🎨 Visual Design ("Runner Vision")

- **Stark High-Contrast Palette**: Brilliant white/light-grey architecture against a vibrant azure sky.
- **Runner Vision RED (`#F5192D`)**: Highlights actionable parkour elements:
  - Red springboard launch ramps.
  - Red low ventilation pipes (must slide under).
  - Red vault barriers and catwalk edges.
  - Red wall-running billboards over fatal drops.
- **Dynamic Camera Juice**:
  - FOV scales dynamically from 75° at base speed up to 95°+ at top sprint velocity.
  - Camera banking/tilt during wall-runs and sharp strafing.
  - Head bobbing synchronized to footstep cadence.
  - 360° rotational camera animation during landing rolls.
  - High-speed radial speed lines overlay.

---

## 🔊 Procedural Audio

Zero external audio files required. All sound effects are procedurally synthesized into memory at launch:
- Dynamic wind rush whose volume and pitch track velocity in real-time.
- Variable-cadence footstep thuds.
- Jump whooshes, slide friction hisses, and wall-run hums.
- Heavy vs. soft landing impacts.
- Stunt combo chimes.

---

## 🛠️ Build & Run

### Quick Launch:
```bash
./run.sh
```

### Manual CMake Build:
```bash
cmake -B build -DCUSTOMIZE_BUILD=ON -DBUILD_EXAMPLES=OFF
cmake --build build -j$(nproc)
./build/EdgeRacer
```
