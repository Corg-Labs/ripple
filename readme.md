# Water Ripple Simulation in C

A real-time 2-D wave-equation simulation — raindrops fall on a virtual water surface, sending concentric ripples that interfere, reflect, and damp out. Rendered as coloured cells via SDL2.

Click anywhere on the window to drop your own pebble.

---

# Features

- Discretised 2‑D wave equation on an 80 × 40 grid
- Two-buffer ping-pong architecture for cache-friendly updates
- Damping coefficient (0.96) for realistic energy dissipation
- Random raindrops every 25 frames
- Mouse-click interaction — drop a pebble at any cell
- True 24‑bit color rendering via SDL2 framebuffer
- 960 × 480 window (12 px per cell)
- Continuous colour gradient: dark navy → teal → cyan-white
- Keyboard and Ctrl-C quit handling
- Written entirely in C

---

# How It Works

Two height buffers (`prev` and `cur`) store the water surface. Each frame, every interior cell computes a finite-difference wave equation from its four orthogonal neighbours — the new height equals half the neighbour sum minus the cell's own previous height. A 0.96 damping multiplier slowly drains energy. The buffers are then swapped and the result is rendered as a smooth blue gradient.

---

# Tutorial / Wave Simulation

## 1. The Height Buffers

The water surface is a 2‑D grid of `double` values. Two global arrays hold the **current** and **previous** frames:

```c
#define W  80
#define H  40

static double cur[H][W], prev[H][W];
```

`prev` holds the height from the last tick; `cur` is overwritten with newly computed heights. Both start at zero (flat surface, C static storage).

---

## 2. Seeding Raindrops

Every 25 frames, a random cell (away from the border) is forced to height 8.0 in `prev`:

```c
if (frame % 25 == 0) {
    int rx = 4 + rand() % (W - 8);
    int ry = 4 + rand() % (H - 8);
    prev[ry][rx] = 8.0;
}
```

Setting only `prev` (not `cur`) means the disturbance enters the wave equation naturally on the very next physics step.

A **mouse click** does the same thing at the clicked cell, letting you interact directly:

```c
if (ev.type == SDL_MOUSEBUTTONDOWN) {
    int gx = ev.button.x / CELL;
    int gy = ev.button.y / CELL;
    prev[gy][gx] = 10.0;
}
```

---

## 3. The Wave Equation

The core is a discretised 2‑D wave propagator applied to every interior cell:

```c
for (int y = 1; y < H - 1; y++)
    for (int x = 1; x < W - 1; x++) {
        double v = (prev[y-1][x] + prev[y+1][x] +
                    prev[y][x-1] + prev[y][x+1]) * 0.5 - cur[y][x];
        cur[y][x] = v * 0.96;
    }
```

The formula is a standard finite-difference wave equation: the new height equals the average of the four orthogonal neighbours (scaled by 2) minus the height at the same cell one step earlier. Subtracting the past height gives the oscillatory "spring-back" that makes waves travel outward.

The `* 0.96` damping multiplier drains a little energy every tick so ripples fade realistically rather than echoing forever.

---

## 4. Buffer Swap (Ping-Pong)

After the physics update, the two buffers are exchanged:

```c
for (int y = 0; y < H; y++)
    for (int x = 0; x < W; x++) {
        double t = prev[y][x];
        prev[y][x] = cur[y][x];
        cur[y][x] = t;
    }
```

This two-buffer pattern avoids allocating a third scratch array and keeps the pipeline cache-friendly. `prev` always holds the most recent heights; `cur` is free to be overwritten next frame.

---

## 5. Color Mapping

Each cell's amplitude `v` is mapped to a continuous blue-to-cyan gradient:

```c
float t = v * 0.15f + 0.5f;
if (t < 0.0f) t = 0.0f;
if (t > 1.0f) t = 1.0f;

color = SDL_MapRGB(fmt,
    (Uint8)(5   + t * 220),    /*  5 → 225 */
    (Uint8)(15  + t * 235),    /* 15 → 250 */
    (Uint8)(40  + t * 215));   /* 40 → 255 */
```

| Amplitude | `t`  | Visual         |
|-----------|------|----------------|
| Strong trough (−6)  | 0.0  | Dark navy  (5, 15, 40)    |
| Flat (0)            | 0.5  | Teal      (115, 132, 147) |
| Strong crest (+6)   | 1.0  | Cyan-white (225, 250, 255) |

---

## 6. SDL2 Cell Rendering

Each grid cell is drawn as a `CELL × CELL` (12 × 12) pixel block:

```c
for (int py = 0; py < CELL; py++)
    for (int px = 0; px < CELL; px++)
        pixels[(y * CELL + py) * pitch + (x * CELL + px)] = color;
```

This gives a clean pixel-art look. At 80 × 40 cells with 12 px cells the window is 960 × 480.

---

## 7. Frame Timing

A 40 fps cap keeps the simulation smooth without busy-waiting:

```c
Uint32 elapsed = SDL_GetTicks() - frame_start;
if (elapsed < 25) SDL_Delay(25 - elapsed);
```

---

# Build

```
git clone <this-repo>
cd ripple
make
```

Or manually:

```
gcc ripple.c -o ripple -lm $(sdl2-config --cflags --libs)
```

**Dependencies:** SDL2 and a C compiler (`gcc`/`clang`).

Install SDL2 via Homebrew:

```
brew install sdl2
```

Or apt:

```
sudo apt install libsdl2-dev
```

---

# Run

```
./ripple
```

Controls:
- **Mouse click** — drop a pebble at that cell
- **ESC** or **Q** — quit
- **Ctrl-C** — quit (fallback)

---

# Customizing

Edit the constants at the top of `ripple.c`:

- `W`, `H` — grid resolution (more cells = smoother ripples, slower)
- `CELL` — pixel size per cell (larger = blockier)
- Damping factor (`0.96`) — lower = faster dissipation
- Drop interval (`25` frames) and amplitude (`8.0` / `10.0`)
- Color gradient parameters (`5 + t * 220`, etc.)

---

# Concepts Practiced

- Discretised 2‑D wave equation (finite-difference method)
- Two-buffer ping-pong architecture for time-stepping
- Oscillatory dynamics with damping coefficient
- Toroidal boundary conditions (fixed border)
- Mouse-to-grid coordinate mapping for interaction
- Continuous colour mapping from scalar amplitude
- SDL2 surface/pixel-buffer graphics
- Real-time animation with frame-rate capping
- Event-driven windowing: keyboard + mouse

---

# Dependencies

- `SDL.h` — window management, pixel-buffer rendering, input
- `stdio.h` — `stderr` diagnostics
- `stdlib.h` — `NULL`, `EXIT_*` macros
- `string.h` — `memset`
- `math.h` — `fabs`
- `time.h` — `time` for RNG seed
- `signal.h` — `SIGINT` handler
