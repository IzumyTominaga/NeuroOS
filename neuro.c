#include "neuro.h"
#include "proc.h"
#include "vga.h"

static neuro_state_t state;
static uint32_t prev_ticks[8];

void neuro_init(uint32_t window_ticks) {
    state.window_ticks  = window_ticks;
    state.last_window   = 0;
    state.total_windows = 0;
    state.anomalies     = 0;
    for (int i = 0; i < 8; i++) {
        state.metrics[i].pid        = i;
        state.metrics[i].ticks_last = 0;
        state.metrics[i].cpu_pct    = 0;
        state.metrics[i].starvation = 0;
        state.metrics[i].health     = HEALTH_OK;
        prev_ticks[i] = 0;
    }
    vga_set_color(0x0B, 0x00);
    vga_puts("NeuroEngine: iniciado. Ventana=");
    char buf[8]; int i=6; buf[7]=0;
    uint32_t n=window_ticks;
    do { buf[--i]='0'+(n%10); n/=10; } while(n&&i>0);
    vga_puts(buf+i);
    vga_puts(" ticks\n");
}

void neuro_tick(void) {
    uint32_t now = get_system_ticks();

    if (now - state.last_window < state.window_ticks) return;
    state.last_window   = now;
    state.total_windows++;

    proc_info_t info[8];
    int count = proc_get_info_all(info, 8);

    for (int i = 0; i < count; i++) {
        proc_metric_t* m = &state.metrics[i];
        m->pid = info[i].pid;

        uint32_t delta = info[i].ticks - prev_ticks[i];
        prev_ticks[i]  = info[i].ticks;

        m->cpu_pct = (delta * 100) / state.window_ticks;
        if (m->cpu_pct > 100) m->cpu_pct = 100;

        m->ticks_last = delta;

        if (info[i].state == 1 && delta == 0) {
            m->starvation++;
        } else {
            m->starvation = 0;
        }

        health_t prev_health = m->health;

        if (m->cpu_pct > 90) {
            m->health = HEALTH_CRITICAL;
        } else if (m->cpu_pct > 70 || m->starvation > 3) {
            m->health = HEALTH_WARN;
        } else {
            m->health = HEALTH_OK;
        }

        if (m->health > prev_health)
            state.anomalies++;

        if (m->starvation >= 3) {
            proc_boost_priority(m->pid, 3);
            state.anomalies++;
            vga_set_color(0x0D, 0x00);
        }
    }
}

static void print_num(uint32_t n) {
    char buf[12]; int i=10; buf[11]=0;
    do { buf[--i]='0'+(n%10); n/=10; } while(n&&i>0);
    vga_puts(buf+i);
}

static void print_bar(uint32_t pct, uint8_t color) {
    int filled = (pct * 10) / 100;
    vga_set_color(0x08, 0x00); vga_puts("[");
    for (int i = 0; i < 10; i++) {
        vga_set_color(i < filled ? color : 0x08, 0x00);
        vga_puts(i < filled ? "#" : " ");
    }
    vga_set_color(0x08, 0x00); vga_puts("]");
}

void neuro_print_status(void) {
    proc_info_t info[8];
    int count = proc_get_info_all(info, 8);

    vga_set_color(0x0E, 0x00);
    vga_puts("\n[ NeuroEngine Status ]\n");
    vga_set_color(0x08, 0x00);
    vga_puts("Ventanas: "); print_num(state.total_windows);
    vga_puts("  Anomalias: "); print_num(state.anomalies);
    vga_puts("  Ticks: "); print_num(get_system_ticks());
    vga_puts("\n\n");

    vga_set_color(0x0F, 0x00);
    vga_puts("PID  NOMBRE        CPU%  SALUD     STARV\n");
    vga_set_color(0x08, 0x00);
    vga_puts("---  ------        ----  -----     -----\n");

    for (int i = 0; i < count; i++) {
        if (info[i].state == 0) continue;
        proc_metric_t* m = &state.metrics[i];

        uint8_t hcol = (m->health == HEALTH_OK)       ? 0x0A :
                       (m->health == HEALTH_WARN)      ? 0x0E : 0x0C;

        vga_set_color(hcol, 0x00);
        vga_puts("  "); print_num(info[i].pid); vga_puts("  ");

        int j = 0;
        while (info[i].name[j]) {
            char s[2] = {info[i].name[j], 0};
            vga_puts(s); j++;
        }
        while (j++ < 12) vga_puts(" ");
        vga_puts("  ");

        print_num(m->cpu_pct); vga_puts("%");
        if (m->cpu_pct < 10) vga_puts("  ");
        else if (m->cpu_pct < 100) vga_puts(" ");
        vga_puts("  ");

        print_bar(m->cpu_pct, hcol);
        vga_puts("  ");

        const char* health_str[] = {"OK      ", "WARN    ", "CRITICO "};
        vga_puts(health_str[m->health]);
        vga_puts("  ");
        print_num(m->starvation);
        vga_puts("\n");
    }
}

neuro_state_t* neuro_get_state(void) { return &state; }
