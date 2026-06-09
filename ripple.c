/*
 * ripple.c - Water ripple height-field simulation (SDL2)
 *
 * Discretised 2-D wave equation on a toroidal grid, rendered
 * as coloured cells via SDL2.  Click anywhere to drop a pebble.
 *
 * Build:
 *   gcc ripple.c -o ripple -lm $(sdl2-config --cflags --libs)
 * Run:
 *   ./ripple
 *
 * Controls:
 *   Click  — drop a pebble at that cell
 *   ESC/Q  — quit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <SDL.h>

#define W      80
#define H      40
#define CELL   12
#define WIN_W  (W * CELL)
#define WIN_H  (H * CELL)

static volatile int running = 1;
static void handle_sigint(int s) { (void)s; running = 0; }

static double cur[H][W], prev[H][W];

int main(void)
{
    signal(SIGINT, handle_sigint);
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Water Ripple",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Surface *surf = SDL_GetWindowSurface(win);
    SDL_PixelFormat *fmt = surf->format;
    int pitch = surf->pitch / 4;

    int frame = 0;

    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = 0;
            if (ev.type == SDL_KEYDOWN &&
                (ev.key.keysym.sym == SDLK_ESCAPE ||
                 ev.key.keysym.sym == SDLK_q))
                running = 0;
            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                int gx = ev.button.x / CELL;
                int gy = ev.button.y / CELL;
                if (gx >= 0 && gx < W && gy >= 0 && gy < H)
                    prev[gy][gx] = 10.0;
            }
        }

        /* ── seed random drops ── */
        if (frame % 25 == 0) {
            int rx = 4 + rand() % (W - 8);
            int ry = 4 + rand() % (H - 8);
            prev[ry][rx] = 8.0;
        }

        /* ── wave equation ── */
        for (int y = 1; y < H - 1; y++)
            for (int x = 1; x < W - 1; x++) {
                double v = (prev[y-1][x] + prev[y+1][x] +
                            prev[y][x-1] + prev[y][x+1]) * 0.5 - cur[y][x];
                cur[y][x] = v * 0.96;
            }

        /* ── ping-pong swap ── */
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++) {
                double t = prev[y][x];
                prev[y][x] = cur[y][x];
                cur[y][x] = t;
            }

        /* ── render ── */
        if (SDL_MUSTLOCK(surf))
            SDL_LockSurface(surf);

        Uint32 *pixels = (Uint32 *)surf->pixels;

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                float t = (float)(prev[y][x] * 0.15f + 0.5f);
                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
                Uint32 color = SDL_MapRGB(fmt,
                    (Uint8)(5   + t * 220),
                    (Uint8)(15  + t * 235),
                    (Uint8)(40  + t * 215));
                for (int py = 0; py < CELL; py++)
                    for (int px = 0; px < CELL; px++)
                        pixels[(y * CELL + py) * pitch + (x * CELL + px)] = color;
            }
        }

        if (SDL_MUSTLOCK(surf))
            SDL_UnlockSurface(surf);

        SDL_UpdateWindowSurface(win);

        frame++;

        Uint32 elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < 25)
            SDL_Delay(25 - elapsed);
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
