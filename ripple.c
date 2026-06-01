/* ripple.c - Water-ripple height-field simulation with random drops.
 * Build: gcc ripple.c -o ripple -lm */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define W 80
#define H 40

static volatile int running = 1;
static void stop(int s){ (void)s; running = 0; }

static double cur[H][W], prev[H][W];

int main(void){
    signal(SIGINT, stop);
    srand((unsigned)time(NULL));
    const char *grad = " .:-=+*#%@";
    printf("\033[2J\033[?25l");
    int frame=0;
    while (running){
        if (frame % 25 == 0){
            int rx=4+rand()%(W-8), ry=4+rand()%(H-8);
            prev[ry][rx] = 8.0;
        }
        for (int y=1;y<H-1;y++)
            for (int x=1;x<W-1;x++){
                double v = (prev[y-1][x]+prev[y+1][x]+prev[y][x-1]+prev[y][x+1])/2.0 - cur[y][x];
                v *= 0.96;                 /* damping */
                cur[y][x] = v;
            }
        for (int y=0;y<H;y++) for (int x=0;x<W;x++){ double t=prev[y][x]; prev[y][x]=cur[y][x]; cur[y][x]=t; }
        printf("\033[H");
        for (int y=0;y<H;y++){
            for (int x=0;x<W;x++){
                double v = fabs(prev[y][x]);
                int idx = (int)(v*1.2); if(idx>9)idx=9;
                putchar(grad[idx]);
            }
            putchar('\n');
        }
        fflush(stdout);
        frame++;
        usleep(40000);
    }
    printf("\033[0m\033[?25h\033[2J\033[H");
    return 0;
}
