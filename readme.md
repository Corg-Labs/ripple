# Water Ripple

Water-ripple height-field simulation, in C.

A small, self-contained demo written in **pure C** — no external libraries,
just the standard library and POSIX. Part of the [Corg-Labs](https://github.com/Corg-Labs)
collection of single-file C programs.

---

## How It Works

1. Two height buffers store the previous and current surface
2. Each cell averages its neighbours minus its own past height
3. A damping factor makes ripples fade realistically
4. Random drops perturb the surface to keep it lively

---

# Tutorial

This tutorial walks through the complete source of `ripple.c` step-by-step, explaining the data structures, physics update, buffer-swap, and terminal rendering that make the simulation work.

## 1. The Two Height Buffers

The simulation stores the water surface as a 2-D grid of `double` values. Two global arrays are kept — one for the **current** frame and one for the **previous** frame:

```c
#define W 80
#define H 40

static double cur[H][W], prev[H][W];
```

`prev[y][x]` holds the height from the last tick; `cur[y][x]` is overwritten with the newly computed height each tick. Both arrays start at zero (C static storage), representing a perfectly flat surface.

## 2. Seeding Drops

Every 25 frames a raindrop is simulated by forcing a single cell in `prev` to a large positive height (8.0). The cell is chosen at random, kept a few cells away from the border so the first ripple ring has room to travel:

```c
if (frame % 25 == 0){
    int rx=4+rand()%(W-8), ry=4+rand()%(H-8);
    prev[ry][rx] = 8.0;
}
```

Setting only `prev` (not `cur`) means the disturbance enters the wave equation naturally on the very next physics step.

## 3. The Wave Equation

The core of the simulation is a discretised 2-D wave equation applied to every interior cell (the one-cell border is left at zero as a fixed boundary condition):

```c
for (int y=1;y<H-1;y++)
    for (int x=1;x<W-1;x++){
        double v = (prev[y-1][x]+prev[y+1][x]+prev[y][x-1]+prev[y][x+1])/2.0 - cur[y][x];
        v *= 0.96;                 /* damping */
        cur[y][x] = v;
    }
```

The formula is the standard finite-difference wave propagator: the new height equals the average of the four orthogonal neighbours (scaled by 2) minus the height at the same cell one step earlier. Subtracting the past height gives the oscillatory "spring-back" that makes waves travel outward. The `* 0.96` damping factor drains a little energy every tick so ripples fade rather than echo forever.

## 4. The Buffer Swap

After the physics update, `prev` and `cur` are exchanged so that `prev` always holds the most recently computed heights and `cur` is free to be overwritten in the next iteration:

```c
for (int y=0;y<H;y++) for (int x=0;x<W;x++){ double t=prev[y][x]; prev[y][x]=cur[y][x]; cur[y][x]=t; }
```

This two-buffer (ping-pong) pattern avoids allocating a third scratch array and keeps the physics update cache-friendly.

## 5. ASCII Rendering

The terminal is cleared to the home position with a single ANSI escape (`\033[H`) and each cell is mapped to a character from a 10-level greyscale gradient:

```c
const char *grad = " .:-=+*#%@";
...
double v = fabs(prev[y][x]);
int idx = (int)(v*1.2); if(idx>9)idx=9;
putchar(grad[idx]);
```

`fabs` is used so that both crests (positive) and troughs (negative) of the wave show as bright glyphs — only the amplitude matters for display, not the sign. The scale factor `1.2` stretches the [0, 8] amplitude range across all 10 gradient levels.

## 6. Frame Timing and Signal Handling

A `usleep(40000)` call at the end of each tick produces roughly 25 frames per second (40 ms per frame). The simulation runs until SIGINT (Ctrl-C) sets the `running` flag to 0:

```c
static volatile int running = 1;
static void stop(int s){ (void)s; running = 0; }
...
signal(SIGINT, stop);
...
while (running){ ... usleep(40000); }
printf("\033[0m\033[?25h\033[2J\033[H");
```

On exit the cursor is restored (`\033[?25h`), colours are reset (`\033[0m`), and the screen is cleared so the terminal is left in a clean state.

---

## Build

```
gcc ripple.c -o ripple -lm
```

## Run

```
./ripple
```

## Controls

Press **Ctrl-C** to quit.
