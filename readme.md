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
