# VECTOR STORM - Roadmap

What the team has built so far, and ideas for later.

## Done

| Feature | Where |
| :--- | :--- |
| Code split into modules (`src/Audio`, `src/Graphics`, `src/Entities`, `src/Core`) | `src/`, `include/` |
| Frame-rate independent game loop (fixed 60 Hz timestep) | `update()` / `simulateFrame()` in `main.cpp` |
| Full 2D movement with inertia | `simulateFrame()` |
| Tactical dash on `Shift` (0.3 s invulnerability, 1.2 s cooldown) | `tryDash()` |
| `F11` fullscreen toggle | `toggleFullscreen()` |
| Persistent top-10 leaderboard + LEADERBOARD screen | `src/Core/SaveSystem.cpp`, `drawLeaderboard()` |
| Persistent settings (volumes, screen shake, ship colour) | `SaveSystem::saveSettings()` |
| Particle pool with explosions, engine trails and smoke | `src/Graphics/ParticleSystem.cpp` |
| Floating score / combat text | `addFloatingText()` |
| Screen shake on enemy explosions, not only on player hits | `triggerShake()` calls in `simulateFrame()` |
| Three enemy types (Scout, Interceptor, Bomber) | `spawnWave()` |
| Radar minimap (F1) and debug views (F1 FPS panel, F4 paths + hitboxes) | `drawRadar()`, `drawCgLabPanel()`, `drawDebugOverlay()` |
| Standalone Windows build (only `libfreeglut.dll` needed) | `build.bat`, `Makefile`, `CMakeLists.txt` |
| Linux build (silent audio) | `Makefile`, `CMakeLists.txt` |

## Ideas not started

- Three playable ship types (fast / heavy / missile) chosen before a mission
- 2-player local co-op (shared combo, revive capsule)
- Multi-part bosses (destructible turrets, shield core) and an enrage phase below 25% HP
- Hit-stop (30-50 ms freeze) on big impacts
- Scrap pickups and a between-sector upgrade hangar
- Circular cooldown timers for power-ups in the HUD
- Gamepad / XInput support and key rebinding
- Low-pass "muffled" audio filter when paused or on the last heart
- Sound on Linux / macOS (the synthesiser currently outputs through Windows WinMM only)

## Team workflow

1. Branch from `main` (`feature/<name>`), keep commits small.
2. Build and play with `.\build.bat run` (or `make run`) before opening a pull request.
3. Get one teammate's review, then merge into `main`.
