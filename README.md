# 🪢 Rope Simulation

A real-time physics-based rope simulation built with **C++ and SFML 3.0**, using Verlet integration and constraint solving.

![Rope Simulation](screenshot.png)

## ✨ Features

- **4 ropes** with different configurations (hanging, pinned both ends, diagonal, pendulum)
- Realistic physics using **Verlet integration** with iterative constraint solving
- **Drag** any point on a rope with the mouse
- **Cut** ropes with right-click
- **Pin/Unpin** any point (P key near a point)
- **Wind simulation** — ropes sway dynamically
- **Low gravity mode** for floaty behavior
- Tension-based color rendering (rope color shifts under stress)

## 🎮 Controls

| Key / Mouse | Action |
|-------------|--------|
| `LMB drag` | Grab and move rope points |
| `RMB click` | Cut rope at that point |
| `P` | Pin / Unpin nearest point |
| `W` | Toggle wind |
| `G` | Toggle low gravity |
| `Space` | Reset all ropes |
| `Esc` | Quit |

## 🛠️ Build from Source

**Requirements:** C++17, CMake 3.16+, SFML 3.0, vcpkg, MinGW

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake \
         -DSFML_DIR=C:/vcpkg/installed/x86-mingw-dynamic/share/sfml \
         -G "MinGW Makefiles"
mingw32-make
```

## ▶️ Run (Windows)

1. Download `RopeSimulation-v1.0-Windows.zip`
2. Extract all files
3. Run `RopeSimulation.exe`

## 🔬 Physics

Uses **Verlet integration** for stable particle simulation with 20 iterations of distance constraint solving per frame, producing realistic rope behavior with catenary curves under gravity.
