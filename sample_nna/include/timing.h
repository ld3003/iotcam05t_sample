
#include <stdint.h>

#ifndef __TIMING__H
#define __TIMING__H



# include <time.h>

# ifndef  CLOCK_MONOTONIC //_RAW
#  define CLOCK_MONOTONIC CLOCK_REALTIME
# endif
#endif

static
uint64_t nanotimer() {
    static int ever = 0;
    struct timespec t;
    if (!ever) {
        if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) {
            return 0;
        }
        ever = 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &t); 
    return (t.tv_sec * (uint64_t) 1e9) + t.tv_nsec;
}


static double now() {

    static uint64_t epoch = 0;
    if (!epoch) {
        epoch = nanotimer();
    }
    return (nanotimer() - epoch) / 1e9;
};

double calcElapsed(double start, double end) {
    double took = -start;
    return took + end;
}
