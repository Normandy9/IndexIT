#ifndef BENCHMARK_H
#define BENCHMARK_H

void benchmark_init(void);
void benchmark_cleanup(void);

void timer_start(void);
double timer_stop_ms(void);

#endif
