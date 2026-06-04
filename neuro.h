#ifndef NEURO_H
#define NEURO_H
#include <stdint.h>

typedef enum {
    HEALTH_OK      = 0,
    HEALTH_WARN    = 1,
    HEALTH_CRITICAL= 2
} health_t;

typedef struct {
    uint32_t pid;
    uint32_t ticks_last;
    uint32_t cpu_pct;
    uint32_t starvation;
    health_t health;
} proc_metric_t;

typedef struct {
    uint32_t     window_ticks;
    uint32_t     last_window;
    uint32_t     total_windows;
    uint32_t     anomalies;
    proc_metric_t metrics[8];
} neuro_state_t;

void neuro_init(uint32_t window_ticks);
void neuro_tick(void);
void neuro_print_status(void);
neuro_state_t* neuro_get_state(void);

#endif
